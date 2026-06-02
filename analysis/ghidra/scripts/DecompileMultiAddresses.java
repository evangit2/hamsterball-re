import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.io.File;
import java.io.FileWriter;

public class DecompileMultiAddresses extends GhidraScript {
    public void run() throws Exception {
        // args format: addr1:out1,addr2:out2,...
        String argStr = getScriptArgs()[0];
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        
        String[] pairs = argStr.split(",");
        for (String pair : pairs) {
            String[] parts = pair.split(":");
            if (parts.length != 2) continue;
            String addrStr = parts[0].trim();
            String outPath = parts[1].trim();
            try {
                Address addr = currentProgram.parseAddress(addrStr);
                Function fn = getFunctionAt(addr);
                if (fn == null) {
                    println("WARN: no function at " + addrStr);
                    continue;
                }
                DecompileResults r = di.decompileFunction(fn, 300, null);
                String c = r.getDecompiledFunction().getC();
                FileWriter fw = new FileWriter(new File(outPath));
                fw.write("// " + addrStr + "\n");
                fw.write(c);
                fw.close();
                println("OK: " + addrStr + " -> " + outPath);
            } catch (Exception e) {
                println("ERR " + addrStr + ": " + e.getMessage());
            }
        }
        di.dispose();
    }
}
