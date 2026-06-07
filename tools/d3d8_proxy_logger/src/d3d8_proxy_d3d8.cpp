//// d3d8_proxy_d3d8.cpp — IDirect3D8 interface wrapper + Direct3DCreate8 export

#include "d3d8_proxy.h"

//// Forward declarations for Device proxy methods (defined in d3d8_proxy_device.cpp)
extern ProxyDevice8* proxy_device_create(IDirect3DDevice8* real);
extern void proxy_device_attach_vtable(ProxyDevice8* p);

//// === IDirect3D8 wrapper === ////

class ProxyD3D8 : public IDirect3D8 {
    IDirect3D8* m_real;
    ULONG       m_ref;
public:
    ProxyD3D8(IDirect3D8* real) : m_real(real), m_ref(1) {}

    //// IUnknown ////
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) {
        proxy_log("d3d8", "QueryInterface");
        HRESULT hr = m_real->QueryInterface(riid, ppv);
        return hr;
    }
    STDMETHOD_(ULONG,AddRef)() {
        ULONG r = m_real->AddRef();
        proxy_log("d3d8", "AddRef -> %u", (unsigned)r);
        return ++m_ref;
    }
    STDMETHOD_(ULONG,Release)() {
        ULONG r = m_real->Release();
        ULONG my = --m_ref;
        proxy_log("d3d8", "Release -> real=%u proxy=%u", (unsigned)r, (unsigned)my);
        if (my == 0) { delete this; return 0; }
        return my;
    }

    //// IDirect3D8 ////
    STDMETHOD(RegisterSoftwareDevice)(void* pfnInit) {
        proxy_log("d3d8", "RegisterSoftwareDevice(%p)", pfnInit);
        return m_real->RegisterSoftwareDevice(pfnInit);
    }
    STDMETHOD_(UINT, GetAdapterCount)() {
        UINT r = m_real->GetAdapterCount();
        proxy_log("d3d8", "GetAdapterCount -> %u", (unsigned)r);
        return r;
    }
    STDMETHOD(GetAdapterIdentifier)(UINT a, DWORD flags, D3DADAPTER_IDENTIFIER8* id) {
        HRESULT hr = m_real->GetAdapterIdentifier(a, flags, id);
        proxy_log("d3d8", "GetAdapterIdentifier(%u,0x%X) -> 0x%X", (unsigned)a,(unsigned)flags,(unsigned)hr);
        return hr;
    }
    STDMETHOD_(UINT, GetAdapterModeCount)(UINT a) {
        UINT r = m_real->GetAdapterModeCount(a);
        proxy_log("d3d8", "GetAdapterModeCount(%u) -> %u", (unsigned)a,(unsigned)r);
        return r;
    }
    STDMETHOD(EnumAdapterModes)(UINT a, UINT m, D3DDISPLAYMODE* dm) {
        HRESULT hr = m_real->EnumAdapterModes(a, m, dm);
        proxy_log("d3d8", "EnumAdapterModes(%u,%u) %ux%u fmt=%s -> 0x%X",
            (unsigned)a,(unsigned)m,(unsigned)(dm?dm->Width:0),(unsigned)(dm?dm->Height:0),
            d3dformat_str(dm?dm->Format:D3DFMT_UNKNOWN),(unsigned)hr);
        return hr;
    }
    STDMETHOD(GetAdapterDisplayMode)(UINT a, D3DDISPLAYMODE* dm) {
        HRESULT hr = m_real->GetAdapterDisplayMode(a, dm);
        proxy_log("d3d8", "GetAdapterDisplayMode(%u) %ux%u fmt=%s -> 0x%X",
            (unsigned)a,(unsigned)(dm?dm->Width:0),(unsigned)(dm?dm->Height:0),
            d3dformat_str(dm?dm->Format:D3DFMT_UNKNOWN),(unsigned)hr);
        return hr;
    }
    STDMETHOD(CheckDeviceType)(UINT a, D3DDEVTYPE t, D3DFORMAT disp, D3DFORMAT back, BOOL win) {
        HRESULT hr = m_real->CheckDeviceType(a, t, disp, back, win);
        proxy_log("d3d8", "CheckDeviceType(%u,%s,%s,%s,%d) -> 0x%X",
            (unsigned)a,d3ddevtype_str(t),d3dformat_str(disp),d3dformat_str(back),(int)win,(unsigned)hr);
        return hr;
    }
    STDMETHOD(CheckDeviceFormat)(UINT a, D3DDEVTYPE t, D3DFORMAT adp, DWORD usage, D3DRESOURCETYPE rt, D3DFORMAT cf) {
        HRESULT hr = m_real->CheckDeviceFormat(a, t, adp, usage, rt, cf);
        proxy_log("d3d8", "CheckDeviceFormat(%u,%s,%s,0x%X,%d,%s) -> 0x%X",
            (unsigned)a,d3ddevtype_str(t),d3dformat_str(adp),(unsigned)usage,(int)rt,d3dformat_str(cf),(unsigned)hr);
        return hr;
    }
    STDMETHOD(CheckDeviceMultiSampleType)(UINT a, D3DDEVTYPE t, D3DFORMAT surf, BOOL win, D3DMULTISAMPLE_TYPE ms) {
        HRESULT hr = m_real->CheckDeviceMultiSampleType(a, t, surf, win, ms);
        proxy_log("d3d8", "CheckDeviceMultiSampleType(%u,%s,%s,%d,%s) -> 0x%X",
            (unsigned)a,d3ddevtype_str(t),d3dformat_str(surf),(int)win,d3dmultisample_str(ms),(unsigned)hr);
        return hr;
    }
    STDMETHOD(CheckDepthStencilMatch)(UINT a, D3DDEVTYPE t, D3DFORMAT adp, D3DFORMAT rt, D3DFORMAT ds) {
        HRESULT hr = m_real->CheckDepthStencilMatch(a, t, adp, rt, ds);
        proxy_log("d3d8", "CheckDepthStencilMatch(%u,%s,%s,%s,%s) -> 0x%X",
            (unsigned)a,d3ddevtype_str(t),d3dformat_str(adp),d3dformat_str(rt),d3dformat_str(ds),(unsigned)hr);
        return hr;
    }
    STDMETHOD(GetDeviceCaps)(UINT a, D3DDEVTYPE t, D3DCAPS8* caps) {
        HRESULT hr = m_real->GetDeviceCaps(a, t, caps);
        proxy_log("d3d8", "GetDeviceCaps(%u,%s) maxtex=%u -> 0x%X",
            (unsigned)a,d3ddevtype_str(t),(caps?(unsigned)caps->MaxTextureBlendStages:0),(unsigned)hr);
        return hr;
    }
    STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT a) {
        HMONITOR h = m_real->GetAdapterMonitor(a);
        proxy_log("d3d8", "GetAdapterMonitor(%u) -> %p", (unsigned)a, h);
        return h;
    }

    STDMETHOD(CreateDevice)(UINT a, D3DDEVTYPE t, HWND hwnd, DWORD flags,
                             D3DPRESENT_PARAMETERS* pp, IDirect3DDevice8** ppdev)
    {
        proxy_log("d3d8", "CreateDevice(%u,%s,hwnd=0x%X,flags=0x%X)",
            (unsigned)a,d3ddevtype_str(t),(unsigned)(uintptr_t)hwnd,(unsigned)flags);
        if (pp) {
            proxy_log("d3d8_present", "BackBuffer=%ux%u fmt=%s ds=%s "
                "windowed=%d swap=%s ms=%s fullscreen_refresh=%u",
                (unsigned)pp->BackBufferWidth, (unsigned)pp->BackBufferHeight,
                d3dformat_str(pp->BackBufferFormat),
                d3dformat_str(pp->AutoDepthStencilFormat),
                (int)pp->Windowed, d3dswapeffect_str(pp->SwapEffect),
                d3dmultisample_str(pp->MultiSampleType),
                (unsigned)pp->FullScreen_RefreshRateInHz);
        }

        HRESULT hr = m_real->CreateDevice(a, t, hwnd, flags, pp, ppdev);
        proxy_log("d3d8", "CreateDevice -> 0x%X ppdev=%p", (unsigned)hr, ppdev ? *ppdev : nullptr);

        if (SUCCEEDED(hr) && ppdev && *ppdev) {
            ProxyDevice8* pd = proxy_device_create(*ppdev);
            proxy_device_attach_vtable(pd);
            *ppdev = (IDirect3DDevice8*)&pd->vt;
            proxy_log("d3d8", "CreateDevice wrapped -> proxy=%p", pd);
        }
        return hr;
    }
};

//// Exported Direct3DCreate8 ////

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    proxy_log_open();
    proxy_log("entry", "Direct3DCreate8(SDKVersion=%u)", (unsigned)SDKVersion);

    if (!g_real_Direct3DCreate8) {
        if (!load_real_d3d8()) {
            proxy_log("fatal", "Cannot load real d3d8");
            return nullptr;
        }
    }

    auto fn = (D3DC8_FN)g_real_Direct3DCreate8;
    IDirect3D8* real = fn(SDKVersion);
    if (!real) {
        proxy_log("fatal", "Real Direct3DCreate8 returned NULL");
        return nullptr;
    }

    ProxyD3D8* proxy = new ProxyD3D8(real);
    proxy_log("entry", "Direct3DCreate8 -> proxy=%p real=%p", proxy, real);
    return proxy;
}
