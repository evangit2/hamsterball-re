#!/usr/bin/env python3
"""
Hamsterball Netplay Relay v2

Improved UX:
  - Auto-starts pipe server, waits for DLL
  - Clear status messages for host/guest
  - Host: "Waiting for guest to connect on port X..."
  - Guest: "Connecting to host at IP:port..."
  - Shows connection status clearly
  - Handles disconnect/reconnect gracefully

Architecture:
  - DLL mod (plus_netplay.dll) communicates via named pipe
  - This Python relay handles TCP networking between clients
  - HOST mode: opens TCP server, waits for guest, relays ball state
  - GUEST mode: connects to host TCP server, relays input state

Player assignment:
  - HOST = Player 1 (local), streams P1+P2 ball state to guest
  - GUEST = Player 2 (local), sends P1 input to host, mirrors host's P1 to local P2

Pipe protocol (binary, little-endian):
  Header: [DWORD type] [DWORD length]
  Body: variable based on type
"""

import socket
import struct
import threading
import time
import sys
import os
from enum import IntEnum

PIPE_NAME = r"\\.\pipe\hamsterball_netplay"
DEFAULT_PORT = 5029
PIPE_BUF_SIZE = 4096

class MsgType(IntEnum):
    PING = 0
    PONG = 1
    BALL_STATE = 2
    INPUT_STATE = 3
    FPS_REPORT = 4
    ROLE_SET = 5
    PORT_SET = 6
    STATUS = 7
    HOST_IP = 8

class ConnState(IntEnum):
    OFFLINE = 0
    SEARCHING = 1
    CONNECTING = 2
    CONNECTED = 3
    ERROR = 4

class NetplayRelay:
    def __init__(self):
        self.role = 0
        self.port = DEFAULT_PORT
        self.host_ip = "127.0.0.1"
        self.running = True
        self.pipe_server = None
        self.tcp_socket = None
        self.tcp_client = None
        self.frame_count = 0
        self.local_fps = 0.0
        self.remote_fps = 0.0
        self.last_fps_time = time.time()
        self.fps_frame_count = 0
        self.threads = []
        self.connected_event = threading.Event()

    def log(self, msg):
        ts = time.strftime("%H:%M:%S")
        print(f"[{ts}] {msg}", flush=True)

    def create_pipe(self):
        try:
            import ctypes
            from ctypes import wintypes
            kernel32 = ctypes.windll.kernel32
            PIPE_ACCESS_DUPLEX = 0x00000003
            PIPE_TYPE_BYTE = 0x00000000
            PIPE_READMODE_BYTE = 0x00000000
            PIPE_WAIT = 0x00000000
            PIPE_UNLIMITED_INSTANCES = 255
            INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value
            handle = kernel32.CreateNamedPipeW(
                PIPE_NAME, PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES, 65536, 65536, 0, None)
            if handle == INVALID_HANDLE_VALUE:
                error = ctypes.get_last_error()
                self.log(f"Failed to create named pipe (error {error})")
                return False
            self.pipe_server = handle
            self.log(f"Named pipe created: {PIPE_NAME}")
            return True
        except Exception as e:
            self.log(f"Pipe creation exception: {e}")
            return False

    def wait_for_pipe_connection(self):
        try:
            import ctypes
            kernel32 = ctypes.windll.kernel32
            self.log("Waiting for Hamsterball to connect...")
            connected = kernel32.ConnectNamedPipe(self.pipe_server, None)
            if not connected:
                error = ctypes.get_last_error()
                if error == 535:
                    self.log("Hamsterball already connected!")
                    return True
                self.log(f"ConnectNamedPipe failed (error {error})")
                return False
            self.log("Hamsterball connected!")
            return True
        except Exception as e:
            self.log(f"Pipe connection exception: {e}")
            return False

    def pipe_read(self, size):
        try:
            import ctypes
            kernel32 = ctypes.windll.kernel32
            buf = (ctypes.c_char * size)()
            bytes_read = ctypes.c_ulong(0)
            success = kernel32.ReadFile(
                self.pipe_server, buf, size, ctypes.byref(bytes_read), None)
            if not success:
                return None
            return bytes(buf.raw[:bytes_read.value])
        except Exception as e:
            self.log(f"Pipe read error: {e}")
            return None

    def pipe_write(self, data):
        try:
            import ctypes
            kernel32 = ctypes.windll.kernel32
            buf = (ctypes.c_char * len(data))(*data)
            bytes_written = ctypes.c_ulong(0)
            success = kernel32.WriteFile(
                self.pipe_server, buf, len(data), ctypes.byref(bytes_written), None)
            return success and bytes_written.value == len(data)
        except Exception as e:
            self.log(f"Pipe write error: {e}")
            return False

    def read_pipe_msg(self):
        header = self.pipe_read(8)
        if not header or len(header) < 8:
            return None
        msg_type, data_len = struct.unpack("<II", header)
        body = b""
        if data_len > 0:
            body = self.pipe_read(data_len)
            if not body or len(body) < data_len:
                return None
        return (msg_type, body)

    def send_pipe_msg(self, msg_type, data=b""):
        msg = struct.pack("<II", msg_type, len(data)) + data
        return self.pipe_write(msg)

    def send_status(self, conn_state, remote_fps=0, remote_info=""):
        status = struct.pack("<I", conn_state)
        status += struct.pack("<I", remote_fps)
        info_bytes = remote_info.encode('ascii')[:63]
        info_bytes += b'\x00' * (64 - len(info_bytes))
        status += info_bytes
        self.send_pipe_msg(MsgType.STATUS, status)

    def tcp_pack(self, msg_type, data=b""):
        payload = struct.pack("<II", msg_type, len(data)) + data
        return struct.pack("<H", len(payload)) + payload

    def tcp_send(self, sock, msg_type, data=b""):
        try:
            msg = self.tcp_pack(msg_type, data)
            sock.sendall(msg)
            return True
        except Exception as e:
            self.log(f"TCP send error: {e}")
            return False

    def tcp_recv_msg(self, sock):
        header = b""
        while len(header) < 2:
            chunk = sock.recv(2 - len(header))
            if not chunk:
                return None
            header += chunk
        total_len = struct.unpack("<H", header)[0]
        if total_len > PIPE_BUF_SIZE:
            return None
        payload = b""
        while len(payload) < total_len:
            chunk = sock.recv(total_len - len(payload))
            if not chunk:
                return None
            payload += chunk
        if len(payload) < 8:
            return None
        msg_type, data_len = struct.unpack("<II", payload[:8])
        body = payload[8:8+data_len]
        return (msg_type, body)

    def run_host(self):
        self.log(f"HOST mode: Starting server on 0.0.0.0:{self.port}...")
        self.log(f"HOST: Tell your friend to connect to YOUR IP on port {self.port}")
        try:
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.tcp_socket.bind(("0.0.0.0", self.port))
            self.tcp_socket.listen(1)
            self.tcp_socket.settimeout(1.0)
        except Exception as e:
            self.log(f"HOST: Failed to bind server: {e}")
            self.send_status(ConnState.ERROR, 0, f"Bind error: {e}")
            return

        self.send_status(ConnState.SEARCHING, 0, f"Listening on port {self.port}")
        self.log(f"HOST: Waiting for guest to connect on port {self.port}...")

        while self.running:
            try:
                self.tcp_client, addr = self.tcp_socket.accept()
                self.tcp_client.settimeout(0.5)
                self.log(f"HOST: Guest connected from {addr[0]}:{addr[1]}!")
                self.log(f"HOST: You are Player 1, guest is Player 2")
                self.send_status(ConnState.CONNECTED, 0, f"Guest: {addr[0]}")
                self.connected_event.set()
                self.host_relay_loop()
                self.connected_event.clear()
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    self.log(f"HOST: Accept error: {e}")
                break

        self.send_status(ConnState.OFFLINE, 0, "Host stopped")

    def host_relay_loop(self):
        stop_flag = threading.Event()

        def pipe_to_tcp():
            while not stop_flag.is_set() and self.running:
                msg = self.read_pipe_msg()
                if msg is None:
                    time.sleep(0.01)
                    continue
                msg_type, body = msg
                if msg_type in (MsgType.BALL_STATE, MsgType.FPS_REPORT):
                    self.tcp_send(self.tcp_client, msg_type, body)

        def tcp_to_pipe():
            while not stop_flag.is_set() and self.running:
                try:
                    msg = self.tcp_recv_msg(self.tcp_client)
                    if msg is None:
                        self.log("HOST: Guest disconnected")
                        self.send_status(ConnState.SEARCHING, 0, "Guest disconnected, waiting...")
                        stop_flag.set()
                        return
                    msg_type, body = msg
                    if msg_type == MsgType.INPUT_STATE:
                        self.send_pipe_msg(MsgType.INPUT_STATE, body)
                    elif msg_type == MsgType.FPS_REPORT:
                        if len(body) >= 4:
                            self.remote_fps = struct.unpack("<f", body[:4])[0]
                            self.send_pipe_msg(MsgType.FPS_REPORT, body)
                except socket.timeout:
                    continue
                except Exception as e:
                    self.log(f"HOST: TCP→pipe error: {e}")
                    break

        t1 = threading.Thread(target=pipe_to_tcp, daemon=True)
        t2 = threading.Thread(target=tcp_to_pipe, daemon=True)
        t1.start()
        t2.start()
        t1.join()
        stop_flag.set()
        t2.join(timeout=2)
        try:
            self.tcp_client.close()
        except:
            pass
        self.tcp_client = None

    def run_guest(self):
        self.log(f"GUEST mode: Connecting to host at {self.host_ip}:{self.port}...")
        self.log(f"GUEST: You are Player 2, host is Player 1")
        self.send_status(ConnState.CONNECTING, 0, f"Connecting to {self.host_ip}:{self.port}")

        try:
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_socket.settimeout(5.0)
            self.tcp_socket.connect((self.host_ip, self.port))
            self.tcp_socket.settimeout(0.5)
            self.log(f"GUEST: Connected to host at {self.host_ip}:{self.port}!")
            self.send_status(ConnState.CONNECTED, 0, f"Connected to {self.host_ip}")
            self.connected_event.set()
        except Exception as e:
            self.log(f"GUEST: Failed to connect: {e}")
            self.log(f"GUEST: Make sure the host is running and their firewall allows port {self.port}")
            self.send_status(ConnState.ERROR, 0, f"Connection failed: {e}")
            return

        stop_flag = threading.Event()

        def pipe_to_tcp():
            while not stop_flag.is_set() and self.running:
                msg = self.read_pipe_msg()
                if msg is None:
                    time.sleep(0.01)
                    continue
                msg_type, body = msg
                if msg_type in (MsgType.INPUT_STATE, MsgType.FPS_REPORT):
                    self.tcp_send(self.tcp_socket, msg_type, body)

        def tcp_to_pipe():
            while not stop_flag.is_set() and self.running:
                try:
                    msg = self.tcp_recv_msg(self.tcp_socket)
                    if msg is None:
                        self.log("GUEST: Host disconnected")
                        self.send_status(ConnState.ERROR, 0, "Host disconnected")
                        stop_flag.set()
                        return
                    msg_type, body = msg
                    if msg_type == MsgType.BALL_STATE:
                        self.send_pipe_msg(MsgType.BALL_STATE, body)
                    elif msg_type == MsgType.FPS_REPORT:
                        if len(body) >= 4:
                            self.remote_fps = struct.unpack("<f", body[:4])[0]
                            self.send_pipe_msg(MsgType.FPS_REPORT, body)
                except socket.timeout:
                    continue
                except Exception as e:
                    self.log(f"GUEST: TCP→pipe error: {e}")
                    break

        t1 = threading.Thread(target=pipe_to_tcp, daemon=True)
        t2 = threading.Thread(target=tcp_to_pipe, daemon=True)
        t1.start()
        t2.start()
        t1.join()
        stop_flag.set()
        t2.join(timeout=2)
        try:
            self.tcp_socket.close()
        except:
            pass
        self.tcp_socket = None
        self.connected_event.clear()

    def run(self):
        self.log("=" * 50)
        self.log("Hamsterball Netplay Relay v2")
        self.log("=" * 50)
        self.log("Make sure Hamsterball+ is installed with plus_netplay.dll in Mods/")
        self.log("Start Hamsterball, go to Options, toggle HOST or GUEST mode")
        self.log("")

        if not self.create_pipe():
            self.log("FATAL: Cannot create named pipe. Make sure you're on Windows.")
            return

        pipe_connected = threading.Event()

        def wait_pipe():
            if self.wait_for_pipe_connection():
                pipe_connected.set()

        pipe_thread = threading.Thread(target=wait_pipe, daemon=True)
        pipe_thread.start()

        self.log("Waiting for Hamsterball to start...")
        if not pipe_connected.wait(timeout=60):
            self.log("Timeout: Hamsterball didn't connect within 60 seconds.")
            self.log("Make sure HB+ is installed and plus_netplay.dll is in Mods/")
            return

        self.log("Connected! Set Netplay Mode slider in Options:")
        self.log("  0 = Disabled (default)")
        self.log("  1 = Host (Player 1) — creates server, waits for guest")
        self.log("  2 = Guest (Player 2) — connects to host IP")
        self.log("")

        last_role = 0
        while self.running:
            msg = self.read_pipe_msg()
            if msg:
                msg_type, body = msg
                if msg_type == MsgType.ROLE_SET:
                    new_role = struct.unpack("<I", body[:4])[0] if len(body) >= 4 else 0
                    if new_role != last_role:
                        role_name = {0: "DISABLED", 1: "HOST (Player 1)", 2: "GUEST (Player 2)"}.get(new_role, "UNKNOWN")
                        self.log(f"Role changed: {role_name}")
                        last_role = new_role
                        self.role = new_role
                        if new_role == 1:
                            self.connected_event.clear()
                            threading.Thread(target=self.run_host, daemon=True).start()
                        elif new_role == 2:
                            self.connected_event.clear()
                            threading.Thread(target=self.run_guest, daemon=True).start()
                elif msg_type == MsgType.PORT_SET:
                    self.port = struct.unpack("<I", body[:4])[0] if len(body) >= 4 else DEFAULT_PORT
                    self.log(f"Port set to {self.port}")
                elif msg_type == MsgType.HOST_IP:
                    self.host_ip = body.decode('ascii', errors='ignore').strip('\x00')
                    self.log(f"Host IP set to {self.host_ip}")
                elif msg_type == MsgType.BALL_STATE:
                    if self.tcp_client or (self.tcp_socket and self.role == 2):
                        sock = self.tcp_client if self.tcp_client else self.tcp_socket
                        self.tcp_send(sock, MsgType.BALL_STATE, body)
                elif msg_type == MsgType.INPUT_STATE:
                    if self.tcp_socket:
                        self.tcp_send(self.tcp_socket, MsgType.INPUT_STATE, body)
                elif msg_type == MsgType.FPS_REPORT:
                    if self.tcp_client or (self.tcp_socket and self.role == 2):
                        sock = self.tcp_client if self.tcp_client else self.tcp_socket
                        self.tcp_send(sock, MsgType.FPS_REPORT, body)
            else:
                time.sleep(0.01)

        self.log("Relay shutting down...")

def main():
    relay = NetplayRelay()
    try:
        relay.run()
    except KeyboardInterrupt:
        print("\nShutting down...")
        relay.running = False

if __name__ == "__main__":
    main()
