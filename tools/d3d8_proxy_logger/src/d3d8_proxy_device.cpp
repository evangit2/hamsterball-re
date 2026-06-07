//// d3d8_proxy_device.cpp — IDirect3DDevice8 full proxy vtable
//// All methods log, then forward to real device.

#include "d3d8_proxy.h"
#include <cstddef>
#include <cstdio>

//// ProxyDevice8 helpers ////

ProxyDevice8* proxy_device_create(IDirect3DDevice8* real) {
    ProxyDevice8* p = new ProxyDevice8();
    p->real = real;
    memset(&p->vt, 0, sizeof(p->vt)); // zero-init vtable
    return p;
}

//// Method tables — we assign static C functions to the vtable slots ////

// vtable[0] points to a method that receives the vtable pointer in _this.
// Our real pointer is at offset 0 of ProxyDevice8, and _this points to vt (offset 4).
// So: ProxyDevice8* me = (ProxyDevice8*)((char*)_this - offsetof(ProxyDevice8, vt));
#define GET_REAL \
    ProxyDevice8* me = (ProxyDevice8*)((char*)_this - offsetof(ProxyDevice8, vt)); \
    IDirect3DDevice8* real = me->real

// ===========================================================================
// IUnknown
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_QueryInterface(IDirect3DDevice8* _this, REFIID riid, void** ppv) {
    GET_REAL; proxy_log("device", "QueryInterface");
    return real->QueryInterface(riid, ppv);
}
static ULONG STDMETHODCALLTYPE dev_AddRef(IDirect3DDevice8* _this) {
    GET_REAL; ULONG r = real->AddRef();
    proxy_log("device", "AddRef -> %u", (unsigned)r);
    return r;
}
static ULONG STDMETHODCALLTYPE dev_Release(IDirect3DDevice8* _this) {
    GET_REAL; ULONG r = real->Release();
    proxy_log("device", "Release -> %u", (unsigned)r);
    // NOTE: we don't delete ProxyDevice8 here; the game calls Release when done
    return r;
}

// ===========================================================================
// Capability checks
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_TestCooperativeLevel(IDirect3DDevice8* _this) {
    GET_REAL;
    HRESULT hr = real->TestCooperativeLevel();
    proxy_log("device", "TestCooperativeLevel -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetDeviceCaps(IDirect3DDevice8* _this, D3DCAPS8* caps) {
    GET_REAL;
    HRESULT hr = real->GetDeviceCaps(caps);
    proxy_log("device", "GetDeviceCaps -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetDirect3D(IDirect3DDevice8* _this, IDirect3D8** ppD3D8) {
    GET_REAL;
    HRESULT hr = real->GetDirect3D(ppD3D8);
    proxy_log("device", "GetDirect3D -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetCreationParameters(IDirect3DDevice8* _this, D3DDEVICE_CREATION_PARAMETERS* p) {
    GET_REAL;
    HRESULT hr = real->GetCreationParameters(p);
    proxy_log("device", "GetCreationParameters -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetAvailableTextureMem(IDirect3DDevice8* _this) {
    GET_REAL; // returns mem; not log spam
    return real->GetAvailableTextureMem();
}
static HRESULT STDMETHODCALLTYPE dev_GetDeviceCaps2(IDirect3DDevice8* _this, D3DCAPS8* caps) {
    GET_REAL; // sometimes aliased
    HRESULT hr = real->GetDeviceCaps(caps);
    proxy_log("device", "GetDeviceCaps2 -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetInfo(IDirect3DDevice8* _this, DWORD id, void* data, DWORD size) {
    GET_REAL;
    proxy_log("device", "GetInfo(%u, %u)", (unsigned)id, (unsigned)size);
    return real->GetInfo(id, data, size);
}

// ===========================================================================
// Display / modes
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_GetDisplayMode(IDirect3DDevice8* _this, D3DDISPLAYMODE* dm) {
    GET_REAL;
    if (!dm) { proxy_log("device", "GetDisplayMode(NULL)"); return D3DERR_INVALIDCALL; }
    HRESULT hr = real->GetDisplayMode(dm);
    proxy_log("device", "GetDisplayMode -> %ux%u fmt=%s",
        (unsigned)dm->Width,(unsigned)dm->Height,d3dformat_str(dm->Format));
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetBackBuffer(IDirect3DDevice8* _this, UINT i, D3DBACKBUFFER_TYPE t, IDirect3DSurface8** pp) {
    GET_REAL;
    HRESULT hr = real->GetBackBuffer(i, t, pp);
    proxy_log("device", "GetBackBuffer(%u,%s) -> 0x%X", (unsigned)i, d3dbackbuffer_str(t), (unsigned)hr);
    return hr;
}
/* GetSwapChain not in MinGW d3d8.h — D3D9 only */

// ===========================================================================
// Resource creation
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_CreateTexture(IDirect3DDevice8* _this,
    UINT w, UINT h, UINT lvls, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DTexture8** pptex)
{
    GET_REAL;
    proxy_log("device", "CreateTexture(%ux%u,levels=%u,usage=0x%X,fmt=%s,pool=%s)",
        (unsigned)w,(unsigned)h,(unsigned)lvls,(unsigned)usage,d3dformat_str(fmt),d3dpool_str(pool));
    HRESULT hr = real->CreateTexture(w, h, lvls, usage, fmt, pool, pptex);
    proxy_log("device", "CreateTexture -> 0x%X tex=%p", (unsigned)hr, pptex ? *pptex : nullptr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateVolumeTexture(IDirect3DDevice8* _this,
    UINT w, UINT h, UINT d, UINT lvls, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DVolumeTexture8** pptex)
{
    GET_REAL;
    proxy_log("device", "CreateVolumeTexture(%ux%ux%u,levels=%u,fmt=%s,pool=%s)",
        (unsigned)w,(unsigned)h,(unsigned)d,(unsigned)lvls,d3dformat_str(fmt),d3dpool_str(pool));
    HRESULT hr = real->CreateVolumeTexture(w, h, d, lvls, usage, fmt, pool, pptex);
    proxy_log("device", "CreateVolumeTexture -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateCubeTexture(IDirect3DDevice8* _this,
    UINT sz, UINT lvls, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DCubeTexture8** pptex)
{
    GET_REAL;
    proxy_log("device", "CreateCubeTexture(%u,levels=%u,fmt=%s,pool=%s)",
        (unsigned)sz,(unsigned)lvls,d3dformat_str(fmt),d3dpool_str(pool));
    HRESULT hr = real->CreateCubeTexture(sz, lvls, usage, fmt, pool, pptex);
    proxy_log("device", "CreateCubeTexture -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateVertexBuffer(IDirect3DDevice8* _this,
    UINT len, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer8** ppvb)
{
    GET_REAL;
    proxy_log("device", "CreateVertexBuffer(len=%u,usage=0x%X,fvf=%s(%08X),pool=%s)",
        (unsigned)len,(unsigned)usage,fvf_flags_str(fvf),(unsigned)fvf,d3dpool_str(pool));
    HRESULT hr = real->CreateVertexBuffer(len, usage, fvf, pool, ppvb);
    proxy_log("device", "CreateVertexBuffer -> 0x%X vb=%p", (unsigned)hr, ppvb ? *ppvb : nullptr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateIndexBuffer(IDirect3DDevice8* _this,
    UINT len, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBuffer8** ppib)
{
    GET_REAL;
    proxy_log("device", "CreateIndexBuffer(len=%u,usage=0x%X,fmt=%s,pool=%s)",
        (unsigned)len,(unsigned)usage,d3dformat_str(fmt),d3dpool_str(pool));
    HRESULT hr = real->CreateIndexBuffer(len, usage, fmt, pool, ppib);
    proxy_log("device", "CreateIndexBuffer -> 0x%X ib=%p", (unsigned)hr, ppib ? *ppib : nullptr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateRenderTarget(IDirect3DDevice8* _this,
    UINT w, UINT h, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE ms, BOOL lockable, IDirect3DSurface8** ppsurf)
{
    GET_REAL;
    proxy_log("device", "CreateRenderTarget(%ux%u,fmt=%s,ms=%s,lock=%d)",
        (unsigned)w,(unsigned)h,d3dformat_str(fmt),d3dmultisample_str(ms),(int)lockable);
    HRESULT hr = real->CreateRenderTarget(w, h, fmt, ms, lockable, ppsurf);
    proxy_log("device", "CreateRenderTarget -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateDepthStencilSurface(IDirect3DDevice8* _this,
    UINT w, UINT h, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE ms, IDirect3DSurface8** ppsurf)
{
    GET_REAL;
    proxy_log("device", "CreateDepthStencilSurface(%ux%u,fmt=%s,ms=%s)",
        (unsigned)w,(unsigned)h,d3dformat_str(fmt),d3dmultisample_str(ms));
    HRESULT hr = real->CreateDepthStencilSurface(w, h, fmt, ms, ppsurf);
    proxy_log("device", "CreateDepthStencilSurface -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateImageSurface(IDirect3DDevice8* _this,
    UINT w, UINT h, D3DFORMAT fmt, IDirect3DSurface8** ppsurf)
{
    GET_REAL;
    proxy_log("device", "CreateImageSurface(%ux%u,fmt=%s)",
        (unsigned)w,(unsigned)h,d3dformat_str(fmt));
    HRESULT hr = real->CreateImageSurface(w, h, fmt, ppsurf);
    proxy_log("device", "CreateImageSurface -> 0x%X", (unsigned)hr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_CreateAdditionalSwapChain(IDirect3DDevice8* _this,
    D3DPRESENT_PARAMETERS* pp, IDirect3DSwapChain8** ppsc)
{
    GET_REAL;
    proxy_log("device", "CreateAdditionalSwapChain(%ux%u,fmt=%s)",
        pp?(unsigned)pp->BackBufferWidth:0,pp?(unsigned)pp->BackBufferHeight:0,
        d3dformat_str(pp?pp->BackBufferFormat:D3DFMT_UNKNOWN));
    return real->CreateAdditionalSwapChain(pp, ppsc);
}

// ===========================================================================
// State setting
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_SetTransform(IDirect3DDevice8* _this,
    D3DTRANSFORMSTATETYPE st, const D3DMATRIX* m)
{
    GET_REAL;
    HRESULT hr = real->SetTransform(st, m);
    const char* names[] = {"WORLD","VIEW","PROJECTION","TEXTURE0","TEXTURE1","TEXTURE2","TEXTURE3",
                            "TEXTURE4","TEXTURE5","TEXTURE6","TEXTURE7"};
    const char* name = (unsigned)st < 11 ? names[(unsigned)st] : "?";
    if (m) {
        proxy_log("device", "SetTransform(%s,%s) [%.2f %.2f %.2f %.2f] -> 0x%X",
            name, d3dformat_str((D3DFORMAT)st),
            m->_11,m->_12,m->_13,m->_14,(unsigned)hr);
        log_matrix(name, m);
    } else {
        proxy_log("device", "SetTransform(%s,NULL)", name);
    }
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetTransform(IDirect3DDevice8* _this,
    D3DTRANSFORMSTATETYPE st, D3DMATRIX* m)
{
    GET_REAL; return real->GetTransform(st, m);
}
static HRESULT STDMETHODCALLTYPE dev_MultiplyTransform(IDirect3DDevice8* _this,
    D3DTRANSFORMSTATETYPE st, const D3DMATRIX* m)
{
    GET_REAL;
    proxy_log("device", "MultiplyTransform(%u)", (unsigned)st);
    return real->MultiplyTransform(st, m);
}
static HRESULT STDMETHODCALLTYPE dev_SetRenderState(IDirect3DDevice8* _this,
    D3DRENDERSTATETYPE rs, DWORD val)
{
    GET_REAL;
    HRESULT hr = real->SetRenderState(rs, val);
    proxy_log("device", "SetRenderState(%s, 0x%X/%u)", d3drs_str(rs), (unsigned)val, (unsigned)val);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetRenderState(IDirect3DDevice8* _this,
    D3DRENDERSTATETYPE rs, DWORD* val)
{
    GET_REAL;
    HRESULT hr = real->GetRenderState(rs, val);
    if (val) proxy_log("device", "GetRenderState(%s) -> 0x%X", d3drs_str(rs), (unsigned)*val);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_SetTextureStageState(IDirect3DDevice8* _this,
    DWORD stage, D3DTEXTURESTAGESTATETYPE tss, DWORD val)
{
    GET_REAL;
    HRESULT hr = real->SetTextureStageState(stage, tss, val);
    proxy_log("device", "SetTextureStageState(s%u,%s,0x%X)",
        (unsigned)stage, d3dtss_str(tss), (unsigned)val);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetTextureStageState(IDirect3DDevice8* _this,
    DWORD stage, D3DTEXTURESTAGESTATETYPE tss, DWORD* val)
{
    GET_REAL; return real->GetTextureStageState(stage, tss, val);
}
/* SetSamplerState is D3D9-only — not in D3D8 */
/* GetSamplerState is D3D9-only — not in D3D8 */
static HRESULT STDMETHODCALLTYPE dev_SetRenderTarget(IDirect3DDevice8* _this,
    IDirect3DSurface8* rt, IDirect3DSurface8* ds)
{
    GET_REAL;
    proxy_log("device", "SetRenderTarget(rt=%p,ds=%p)", rt, ds);
    return real->SetRenderTarget(rt, ds);
}
static HRESULT STDMETHODCALLTYPE dev_SetClipPlane(IDirect3DDevice8* _this,
    DWORD idx, const float* plane)
{
    GET_REAL;
    proxy_log("device", "SetClipPlane(%u)", (unsigned)idx);
    return real->SetClipPlane(idx, plane);
}
static HRESULT STDMETHODCALLTYPE dev_GetClipPlane(IDirect3DDevice8* _this,
    DWORD idx, float* plane)
{
    GET_REAL; return real->GetClipPlane(idx, plane);
}

// ===========================================================================
// Lighting / Materials
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_SetLight(IDirect3DDevice8* _this,
    DWORD idx, const D3DLIGHT8* l)
{
    GET_REAL;
    proxy_log("device", "SetLight(%u,type=%u)", (unsigned)idx, l?(unsigned)l->Type:0);
    return real->SetLight(idx, l);
}
static HRESULT STDMETHODCALLTYPE dev_GetLight(IDirect3DDevice8* _this,
    DWORD idx, D3DLIGHT8* l)
{
    GET_REAL; return real->GetLight(idx, l);
}
static HRESULT STDMETHODCALLTYPE dev_LightEnable(IDirect3DDevice8* _this,
    DWORD idx, BOOL en)
{
    GET_REAL;
    proxy_log("device", "LightEnable(%u,%d)", (unsigned)idx, (int)en);
    return real->LightEnable(idx, en);
}
static HRESULT STDMETHODCALLTYPE dev_GetLightEnable(IDirect3DDevice8* _this,
    DWORD idx, BOOL* en)
{
    GET_REAL;
    HRESULT hr = real->GetLightEnable(idx, en);
    proxy_log("device", "GetLightEnable(%u) -> %d", (unsigned)idx, en?*en:-1);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_SetClipStatus(IDirect3DDevice8* _this,
    const D3DCLIPSTATUS8* cs)
{
    GET_REAL; return real->SetClipStatus(cs);
}
static HRESULT STDMETHODCALLTYPE dev_GetClipStatus(IDirect3DDevice8* _this,
    D3DCLIPSTATUS8* cs)
{
    GET_REAL; return real->GetClipStatus(cs);
}
static HRESULT STDMETHODCALLTYPE dev_SetMaterial(IDirect3DDevice8* _this,
    const D3DMATERIAL8* mtrl)
{
    GET_REAL;
    if (mtrl) proxy_log("device", "SetMaterial(diff=%.2f,%.2f,%.2f,%.2f)",
        mtrl->Diffuse.r,mtrl->Diffuse.g,mtrl->Diffuse.b,mtrl->Diffuse.a);
    return real->SetMaterial(mtrl);
}
static HRESULT STDMETHODCALLTYPE dev_GetMaterial(IDirect3DDevice8* _this,
    D3DMATERIAL8* mtrl)
{
    GET_REAL; return real->GetMaterial(mtrl);
}

// ===========================================================================
// Viewport / Scissor
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_SetViewport(IDirect3DDevice8* _this,
    const D3DVIEWPORT8* vp)
{
    GET_REAL;
    if (vp) proxy_log("device", "SetViewport(x=%u,y=%u,w=%u,h=%u,zn=%.4f,zf=%.4f)",
        (unsigned)vp->X,(unsigned)vp->Y,(unsigned)vp->Width,(unsigned)vp->Height,
        vp->MinZ, vp->MaxZ);
    return real->SetViewport(vp);
}
static HRESULT STDMETHODCALLTYPE dev_GetViewport(IDirect3DDevice8* _this,
    D3DVIEWPORT8* vp)
{
    GET_REAL; return real->GetViewport(vp);
}

// ===========================================================================
// Texture / Palettes
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_SetTexture(IDirect3DDevice8* _this,
    DWORD stage, IDirect3DBaseTexture8* tex)
{
    GET_REAL;
    proxy_log("device", "SetTexture(s%u, tex=%p)", (unsigned)stage, tex);
    return real->SetTexture(stage, tex);
}
static HRESULT STDMETHODCALLTYPE dev_SetPaletteEntries(IDirect3DDevice8* _this,
    UINT idx, const PALETTEENTRY* entries)
{
    GET_REAL;
    proxy_log("device", "SetPaletteEntries(%u, entries=%p)", (unsigned)idx, entries);
    return real->SetPaletteEntries(idx, entries);
}
static HRESULT STDMETHODCALLTYPE dev_SetCurrentTexturePalette(IDirect3DDevice8* _this,
    DWORD idx)
{
    GET_REAL;
    proxy_log("device", "SetCurrentTexturePalette(%u)", (unsigned)idx);
    return real->SetCurrentTexturePalette(idx);
}
static HRESULT STDMETHODCALLTYPE dev_GetTexture(IDirect3DDevice8* _this,
    DWORD stage, IDirect3DBaseTexture8** pptex)
{
    GET_REAL; return real->GetTexture(stage, pptex);
}

// ===========================================================================
// Vertex / Index buffers, streams
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_SetStreamSource(IDirect3DDevice8* _this,
    UINT stream, IDirect3DVertexBuffer8* vb, UINT stride)
{
    GET_REAL;
    proxy_log("device", "SetStreamSource(%u, vb=%p, stride=%u)",
        (unsigned)stream, vb, (unsigned)stride);
    return real->SetStreamSource(stream, vb, stride);
}
static HRESULT STDMETHODCALLTYPE dev_GetStreamSource(IDirect3DDevice8* _this,
    UINT stream, IDirect3DVertexBuffer8** ppvb, UINT* stride)
{
    GET_REAL; return real->GetStreamSource(stream, ppvb, stride);
}
static HRESULT STDMETHODCALLTYPE dev_SetIndices(IDirect3DDevice8* _this,
    IDirect3DIndexBuffer8* ib, UINT baseIdx)
{
    GET_REAL;
    proxy_log("device", "SetIndices(ib=%p, base=%u)", ib, (unsigned)baseIdx);
    return real->SetIndices(ib, baseIdx);
}
static HRESULT STDMETHODCALLTYPE dev_GetIndices(IDirect3DDevice8* _this,
    IDirect3DIndexBuffer8** ppib, UINT* baseIdx)
{
    GET_REAL; return real->GetIndices(ppib, baseIdx);
}
static HRESULT STDMETHODCALLTYPE dev_SetVertexShader(IDirect3DDevice8* _this,
    DWORD handle)
{
    GET_REAL;
    proxy_log("device", "SetVertexShader(0x%X)", (unsigned)handle);
    return real->SetVertexShader(handle);
}
static HRESULT STDMETHODCALLTYPE dev_GetVertexShader(IDirect3DDevice8* _this,
    DWORD* handle)
{
    GET_REAL; return real->GetVertexShader(handle);
}
static HRESULT STDMETHODCALLTYPE dev_GetVertexShaderDeclaration(IDirect3DDevice8* _this,
    DWORD handle, void* data, DWORD* size)
{
    GET_REAL; return real->GetVertexShaderDeclaration(handle, data, size);
}
static HRESULT STDMETHODCALLTYPE dev_GetVertexShaderFunction(IDirect3DDevice8* _this,
    DWORD handle, void* data, DWORD* size)
{
    GET_REAL;
    proxy_log("device", "GetVertexShaderFunction(0x%X)", (unsigned)handle);
    return real->GetVertexShaderFunction(handle, data, size);
}
static HRESULT STDMETHODCALLTYPE dev_CreateVertexShader(IDirect3DDevice8* _this,
    const DWORD* decl, const DWORD* func, DWORD* handle, DWORD usage)
{
    GET_REAL;
    proxy_log("device", "CreateVertexShader(usage=0x%X)", (unsigned)usage);
    return real->CreateVertexShader(decl, func, handle, usage);
}
static HRESULT STDMETHODCALLTYPE dev_DeleteVertexShader(IDirect3DDevice8* _this,
    DWORD handle)
{
    GET_REAL;
    proxy_log("device", "DeleteVertexShader(0x%X)", (unsigned)handle);
    return real->DeleteVertexShader(handle);
}
static HRESULT STDMETHODCALLTYPE dev_SetVertexShaderConstant(IDirect3DDevice8* _this,
    DWORD reg, const void* data, DWORD count)
{
    GET_REAL;
    proxy_log("device", "SetVertexShaderConstant(r%u,count=%u)", (unsigned)reg, (unsigned)count);
    return real->SetVertexShaderConstant(reg, data, count);
}
static HRESULT STDMETHODCALLTYPE dev_GetVertexShaderConstant(IDirect3DDevice8* _this,
    DWORD reg, void* data, DWORD count)
{
    GET_REAL; return real->GetVertexShaderConstant(reg, data, count);
}
static HRESULT STDMETHODCALLTYPE dev_SetPixelShader(IDirect3DDevice8* _this,
    DWORD handle)
{
    GET_REAL;
    proxy_log("device", "SetPixelShader(0x%X)", (unsigned)handle);
    return real->SetPixelShader(handle);
}
static HRESULT STDMETHODCALLTYPE dev_GetPixelShader(IDirect3DDevice8* _this,
    DWORD* handle)
{
    GET_REAL; return real->GetPixelShader(handle);
}
static HRESULT STDMETHODCALLTYPE dev_GetPixelShaderFunction(IDirect3DDevice8* _this,
    DWORD handle, void* data, DWORD* size)
{
    GET_REAL; return real->GetPixelShaderFunction(handle, data, size);
}
static HRESULT STDMETHODCALLTYPE dev_CreatePixelShader(IDirect3DDevice8* _this,
    const DWORD* func, DWORD* handle)
{
    GET_REAL;
    proxy_log("device", "CreatePixelShader");
    return real->CreatePixelShader(func, handle);
}
static HRESULT STDMETHODCALLTYPE dev_DeletePixelShader(IDirect3DDevice8* _this,
    DWORD handle)
{
    GET_REAL;
    proxy_log("device", "DeletePixelShader(0x%X)", (unsigned)handle);
    return real->DeletePixelShader(handle);
}

static HRESULT STDMETHODCALLTYPE dev_ProcessVertices(IDirect3DDevice8* _this,
    UINT srcStart, UINT destIdx, UINT count, IDirect3DVertexBuffer8* dest,
    DWORD flags)
{
    GET_REAL;
    proxy_log("device", "ProcessVertices(src=%u,dest=%u,count=%u,flags=0x%X)",
        (unsigned)srcStart,(unsigned)destIdx,(unsigned)count,(unsigned)flags);
    return real->ProcessVertices(srcStart, destIdx, count, dest, flags);
}

// ===========================================================================
// Drawing
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_DrawPrimitive(IDirect3DDevice8* _this,
    D3DPRIMITIVETYPE pt, UINT startVtx, UINT count)
{
    GET_REAL;
    proxy_log("draw", "DrawPrimitive(%s,start=%u,count=%u)",
        d3dprim_str(pt),(unsigned)startVtx,(unsigned)count);
    return real->DrawPrimitive(pt, startVtx, count);
}
static HRESULT STDMETHODCALLTYPE dev_DrawIndexedPrimitive(IDirect3DDevice8* _this,
    D3DPRIMITIVETYPE pt, UINT minIdx, UINT numVtx, UINT startIdx, UINT primCount)
{
    GET_REAL;
    proxy_log("draw", "DrawIndexedPrimitive(%s,min=%u,numVtx=%u,start=%u,count=%u)",
        d3dprim_str(pt),(unsigned)minIdx,(unsigned)numVtx,(unsigned)startIdx,(unsigned)primCount);
    return real->DrawIndexedPrimitive(pt, minIdx, numVtx, startIdx, primCount);
}
static HRESULT STDMETHODCALLTYPE dev_DrawPrimitiveUP(IDirect3DDevice8* _this,
    D3DPRIMITIVETYPE pt, UINT count, const void* data, UINT stride)
{
    GET_REAL;
    proxy_log("draw", "DrawPrimitiveUP(%s,count=%u,data=%p,stride=%u)",
        d3dprim_str(pt),(unsigned)count,data,(unsigned)stride);
    return real->DrawPrimitiveUP(pt, count, data, stride);
}
static HRESULT STDMETHODCALLTYPE dev_DrawIndexedPrimitiveUP(IDirect3DDevice8* _this,
    D3DPRIMITIVETYPE pt, UINT minIdx, UINT numVtx, UINT primCount,
    const void* idxData, D3DFORMAT idxFmt, const void* vtxData, UINT stride)
{
    GET_REAL;
    proxy_log("draw", "DrawIndexedPrimitiveUP(%s,min=%u,numVtx=%u,count=%u,"
        "idxFmt=%s,vtxStride=%u)",
        d3dprim_str(pt),(unsigned)minIdx,(unsigned)numVtx,(unsigned)primCount,
        d3dformat_str(idxFmt),(unsigned)stride);
    return real->DrawIndexedPrimitiveUP(pt, minIdx, numVtx, primCount,
        idxData, idxFmt, vtxData, stride);
}

// ===========================================================================
// Scene management
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_BeginScene(IDirect3DDevice8* _this) {
    GET_REAL;
    proxy_log("scene", "BeginScene");
    return real->BeginScene();
}
static HRESULT STDMETHODCALLTYPE dev_EndScene(IDirect3DDevice8* _this) {
    GET_REAL;
    proxy_log("scene", "EndScene");
    return real->EndScene();
}
static HRESULT STDMETHODCALLTYPE dev_Clear(IDirect3DDevice8* _this,
    DWORD count, const D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
    GET_REAL;
    proxy_log("device", "Clear(count=%u,flags=0x%X,color=0x%08X,z=%.4f,stencil=%u)",
        (unsigned)count,(unsigned)flags,(unsigned)color,z,(unsigned)stencil);
    return real->Clear(count, rects, flags, color, z, stencil);
}
static HRESULT STDMETHODCALLTYPE dev_Present(IDirect3DDevice8* _this,
    const RECT* src, const RECT* dst, HWND hwnd, const RGNDATA* rgndata)
{
    GET_REAL;
    proxy_log("present", "Present");
    return real->Present(src, dst, hwnd, rgndata);
}
static HRESULT STDMETHODCALLTYPE dev_Reset(IDirect3DDevice8* _this,
    D3DPRESENT_PARAMETERS* pp)
{
    GET_REAL;
    proxy_log("device", "Reset(%ux%u,fmt=%s)",
        pp?(unsigned)pp->BackBufferWidth:0,pp?(unsigned)pp->BackBufferHeight:0,
        d3dformat_str(pp?pp->BackBufferFormat:D3DFMT_UNKNOWN));
    return real->Reset(pp);
}
static HRESULT STDMETHODCALLTYPE dev_SetCursorProperties(IDirect3DDevice8* _this,
    UINT xHot, UINT yHot, IDirect3DSurface8* cursorBmp)
{
    GET_REAL;
    proxy_log("device", "SetCursorProperties(%u,%u,%p)", (unsigned)xHot,(unsigned)yHot,cursorBmp);
    return real->SetCursorProperties(xHot, yHot, cursorBmp);
}
static HRESULT STDMETHODCALLTYPE dev_ShowCursor(IDirect3DDevice8* _this,
    BOOL show)
{
    GET_REAL;
    proxy_log("device", "ShowCursor(%d)", (int)show);
    return real->ShowCursor(show);
}

// ===========================================================================
// Depth Stencil Surface
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_GetDepthStencilSurface(IDirect3DDevice8* _this,
    IDirect3DSurface8** pp)
{
    GET_REAL;
    HRESULT hr = real->GetDepthStencilSurface(pp);
    proxy_log("device", "GetDepthStencilSurface -> 0x%X surf=%p", (unsigned)hr, pp?*pp:nullptr);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_SetDepthStencilSurface(IDirect3DDevice8* _this,
    IDirect3DSurface8* ds)
{
    GET_REAL;
    proxy_log("device", "SetDepthStencilSurface(%p)", ds);
    // MinGW d3d8.h bug: SetDepthStencilSurface is named SetRenderTarget(rt,ds)
    // Just log it — the real device method is forwarded via vtable
    // Actually the real vtable call works regardless, bypass MinGW wrapper
    HRESULT hr = real->SetRenderTarget(nullptr, ds);
    return hr;
}

// ===========================================================================
// Misc methods
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_ValidateDevice(IDirect3DDevice8* _this,
    DWORD* passes)
{
    GET_REAL;
    HRESULT hr = real->ValidateDevice(passes);
    proxy_log("device", "ValidateDevice -> 0x%X passes=%u", (unsigned)hr,
        passes?(unsigned)*passes:0);
    return hr;
}
static HRESULT STDMETHODCALLTYPE dev_GetFrontBuffer(IDirect3DDevice8* _this,
    IDirect3DSurface8* surf)  // D3D8 has GetFrontBuffer on device, not swapchain
{
    GET_REAL;
    proxy_log("device", "GetFrontBuffer(%p)", surf);
    return real->GetFrontBuffer(surf);
}
static HRESULT STDMETHODCALLTYPE dev_ResourceManagerDiscardBytes(IDirect3DDevice8* _this,
    DWORD bytes)
{
    GET_REAL;
    proxy_log("device", "ResourceManagerDiscardBytes(%u)", (unsigned)bytes);
    return real->ResourceManagerDiscardBytes(bytes);
}

// ===========================================================================
// Missing stub methods (less commonly used)
// ===========================================================================

static HRESULT STDMETHODCALLTYPE dev_GetRenderTarget(IDirect3DDevice8* _this,
    IDirect3DSurface8** pp)
{
    GET_REAL;
    HRESULT hr = real->GetRenderTarget(pp);
    proxy_log("device", "GetRenderTarget -> 0x%X", (unsigned)hr);
    return hr;
};

/* === Missing stubs — methods declared in MinGW d3d8.h but not yet wrapped === */

static void STDMETHODCALLTYPE dev_SetCursorPosition(IDirect3DDevice8* _this,
    UINT x, UINT y, DWORD flags) {
    GET_REAL; proxy_log("device", "SetCursorPosition(%u,%u)", (unsigned)x, (unsigned)y);
    real->SetCursorPosition(x, y, flags);
}
static HRESULT STDMETHODCALLTYPE dev_GetRasterStatus(IDirect3DDevice8* _this,
    D3DRASTER_STATUS* s) {
    GET_REAL; return real->GetRasterStatus(s);
}
static void STDMETHODCALLTYPE dev_SetGammaRamp(IDirect3DDevice8* _this,
    DWORD flags, const D3DGAMMARAMP* ramp) {
    GET_REAL; proxy_log("device", "SetGammaRamp"); real->SetGammaRamp(flags, ramp);
}
static void STDMETHODCALLTYPE dev_GetGammaRamp(IDirect3DDevice8* _this,
    D3DGAMMARAMP* ramp) {
    GET_REAL; real->GetGammaRamp(ramp);
}
static HRESULT STDMETHODCALLTYPE dev_CopyRects(IDirect3DDevice8* _this,
    IDirect3DSurface8* src, const RECT* srcr, UINT n, IDirect3DSurface8* dst, const POINT* dstp) {
    GET_REAL; proxy_log("device", "CopyRects(n=%u)", (unsigned)n);
    return real->CopyRects(src, srcr, n, dst, dstp);
}
static HRESULT STDMETHODCALLTYPE dev_UpdateTexture(IDirect3DDevice8* _this,
    IDirect3DBaseTexture8* src, IDirect3DBaseTexture8* dst) {
    GET_REAL; proxy_log("device", "UpdateTexture");
    return real->UpdateTexture(src, dst);
}
static HRESULT STDMETHODCALLTYPE dev_BeginStateBlock(IDirect3DDevice8* _this) {
    GET_REAL; proxy_log("device", "BeginStateBlock");
    return real->BeginStateBlock();
}
static HRESULT STDMETHODCALLTYPE dev_EndStateBlock(IDirect3DDevice8* _this, DWORD* token) {
    GET_REAL; proxy_log("device", "EndStateBlock");
    return real->EndStateBlock(token);
}
static HRESULT STDMETHODCALLTYPE dev_ApplyStateBlock(IDirect3DDevice8* _this, DWORD token) {
    GET_REAL; proxy_log("device", "ApplyStateBlock");
    return real->ApplyStateBlock(token);
}
static HRESULT STDMETHODCALLTYPE dev_CaptureStateBlock(IDirect3DDevice8* _this, DWORD token) {
    GET_REAL; proxy_log("device", "CaptureStateBlock");
    return real->CaptureStateBlock(token);
}
static HRESULT STDMETHODCALLTYPE dev_DeleteStateBlock(IDirect3DDevice8* _this, DWORD token) {
    GET_REAL; proxy_log("device", "DeleteStateBlock");
    return real->DeleteStateBlock(token);
}
static HRESULT STDMETHODCALLTYPE dev_CreateStateBlock(IDirect3DDevice8* _this,
    D3DSTATEBLOCKTYPE type, DWORD* token) {
    GET_REAL; proxy_log("device", "CreateStateBlock");
    return real->CreateStateBlock(type, token);
}
static HRESULT STDMETHODCALLTYPE dev_GetPaletteEntries(IDirect3DDevice8* _this,
    UINT idx, PALETTEENTRY* entries) {
    GET_REAL; proxy_log("device", "GetPaletteEntries(%u)", (unsigned)idx);
    return real->GetPaletteEntries(idx, entries);
}
static HRESULT STDMETHODCALLTYPE dev_GetCurrentTexturePalette(IDirect3DDevice8* _this,
    UINT* idx) {
    GET_REAL; proxy_log("device", "GetCurrentTexturePalette");
    return real->GetCurrentTexturePalette(idx);
}
static HRESULT STDMETHODCALLTYPE dev_SetPixelShaderConstant(IDirect3DDevice8* _this,
    DWORD reg, const void* data, DWORD count) {
    GET_REAL; proxy_log("device", "SetPixelShaderConstant(reg=%u)", (unsigned)reg);
    return real->SetPixelShaderConstant(reg, data, count);
}
static HRESULT STDMETHODCALLTYPE dev_GetPixelShaderConstant(IDirect3DDevice8* _this,
    DWORD reg, void* data, DWORD count) {
    GET_REAL; proxy_log("device", "GetPixelShaderConstant(reg=%u)", (unsigned)reg);
    return real->GetPixelShaderConstant(reg, data, count);
}
static HRESULT STDMETHODCALLTYPE dev_DrawRectPatch(IDirect3DDevice8* _this,
    UINT handle, const float* segs, const D3DRECTPATCH_INFO* info) {
    GET_REAL; proxy_log("draw", "DrawRectPatch");
    return real->DrawRectPatch(handle, segs, info);
}
static HRESULT STDMETHODCALLTYPE dev_DrawTriPatch(IDirect3DDevice8* _this,
    UINT handle, const float* segs, const D3DTRIPATCH_INFO* info) {
    GET_REAL; proxy_log("draw", "DrawTriPatch");
    return real->DrawTriPatch(handle, segs, info);
}
static HRESULT STDMETHODCALLTYPE dev_DeletePatch(IDirect3DDevice8* _this, UINT handle) {
    GET_REAL; proxy_log("device", "DeletePatch(0x%X)", (unsigned)handle);
    return real->DeletePatch(handle);
}

//// Vtable slot assignments ////
//// These map to the D3D8 vtable order (0-based) ////

void proxy_device_attach_vtable(ProxyDevice8* p) {
    // We build the vtable as an array of function pointers matching
    // the IDirect3DDevice8 layout. The real object has the same layout.
    // We copy the real vtable first (to get the layout), then override
    // every slot with our logging wrapper.
    //
    // Actually, cleaner approach: just assign our wrappers to every slot.

    void** vt = (void**)&p->vt;

    /* MinGW d3d8.h IDirect3DDevice8 vtable order (97 slots) */
    vt[0]  = (void*)dev_QueryInterface;
    vt[1]  = (void*)dev_AddRef;
    vt[2]  = (void*)dev_Release;
    vt[3]  = (void*)dev_TestCooperativeLevel;
    vt[4]  = (void*)dev_GetAvailableTextureMem;
    vt[5]  = (void*)dev_ResourceManagerDiscardBytes;
    vt[6]  = (void*)dev_GetDirect3D;
    vt[7]  = (void*)dev_GetDeviceCaps;
    vt[8]  = (void*)dev_GetDisplayMode;
    vt[9]  = (void*)dev_GetCreationParameters;
    vt[10] = (void*)dev_SetCursorProperties;
    vt[11] = (void*)dev_SetCursorPosition;
    vt[12] = (void*)dev_ShowCursor;
    vt[13] = (void*)dev_CreateAdditionalSwapChain;
    vt[14] = (void*)dev_Reset;
    vt[15] = (void*)dev_Present;
    vt[16] = (void*)dev_GetBackBuffer;
    vt[17] = (void*)dev_GetRasterStatus;
    vt[18] = (void*)dev_SetGammaRamp;
    vt[19] = (void*)dev_GetGammaRamp;
    vt[20] = (void*)dev_CreateTexture;
    vt[21] = (void*)dev_CreateVolumeTexture;
    vt[22] = (void*)dev_CreateCubeTexture;
    vt[23] = (void*)dev_CreateVertexBuffer;
    vt[24] = (void*)dev_CreateIndexBuffer;
    vt[25] = (void*)dev_CreateRenderTarget;
    vt[26] = (void*)dev_CreateDepthStencilSurface;
    vt[27] = (void*)dev_CreateImageSurface;
    vt[28] = (void*)dev_CopyRects;
    vt[29] = (void*)dev_UpdateTexture;
    vt[30] = (void*)dev_GetFrontBuffer;
    vt[31] = (void*)dev_SetRenderTarget;
    vt[32] = (void*)dev_GetRenderTarget;
    vt[33] = (void*)dev_GetDepthStencilSurface;
    vt[34] = (void*)dev_BeginScene;
    vt[35] = (void*)dev_EndScene;
    vt[36] = (void*)dev_Clear;
    vt[37] = (void*)dev_SetTransform;
    vt[38] = (void*)dev_GetTransform;
    vt[39] = (void*)dev_MultiplyTransform;
    vt[40] = (void*)dev_SetViewport;
    vt[41] = (void*)dev_GetViewport;
    vt[42] = (void*)dev_SetMaterial;
    vt[43] = (void*)dev_GetMaterial;
    vt[44] = (void*)dev_SetLight;
    vt[45] = (void*)dev_GetLight;
    vt[46] = (void*)dev_LightEnable;
    vt[47] = (void*)dev_GetLightEnable;
    vt[48] = (void*)dev_SetClipPlane;
    vt[49] = (void*)dev_GetClipPlane;
    vt[50] = (void*)dev_SetRenderState;
    vt[51] = (void*)dev_GetRenderState;
    vt[52] = (void*)dev_BeginStateBlock;
    vt[53] = (void*)dev_EndStateBlock;
    vt[54] = (void*)dev_ApplyStateBlock;
    vt[55] = (void*)dev_CaptureStateBlock;
    vt[56] = (void*)dev_DeleteStateBlock;
    vt[57] = (void*)dev_CreateStateBlock;
    vt[58] = (void*)dev_SetClipStatus;
    vt[59] = (void*)dev_GetClipStatus;
    vt[60] = (void*)dev_GetTexture;
    vt[61] = (void*)dev_SetTexture;
    vt[62] = (void*)dev_GetTextureStageState;
    vt[63] = (void*)dev_SetTextureStageState;
    vt[64] = (void*)dev_ValidateDevice;
    vt[65] = (void*)dev_GetInfo;
    vt[66] = (void*)dev_SetPaletteEntries;
    vt[67] = (void*)dev_GetPaletteEntries;
    vt[68] = (void*)dev_SetCurrentTexturePalette;
    vt[69] = (void*)dev_GetCurrentTexturePalette;
    vt[70] = (void*)dev_DrawPrimitive;
    vt[71] = (void*)dev_DrawIndexedPrimitive;
    vt[72] = (void*)dev_DrawPrimitiveUP;
    vt[73] = (void*)dev_DrawIndexedPrimitiveUP;
    vt[74] = (void*)dev_ProcessVertices;
    vt[75] = (void*)dev_CreateVertexShader;
    vt[76] = (void*)dev_SetVertexShader;
    vt[77] = (void*)dev_GetVertexShader;
    vt[78] = (void*)dev_DeleteVertexShader;
    vt[79] = (void*)dev_SetVertexShaderConstant;
    vt[80] = (void*)dev_GetVertexShaderConstant;
    vt[81] = (void*)dev_GetVertexShaderDeclaration;
    vt[82] = (void*)dev_GetVertexShaderFunction;
    vt[83] = (void*)dev_SetStreamSource;
    vt[84] = (void*)dev_GetStreamSource;
    vt[85] = (void*)dev_SetIndices;
    vt[86] = (void*)dev_GetIndices;
    vt[87] = (void*)dev_CreatePixelShader;
    vt[88] = (void*)dev_SetPixelShader;
    vt[89] = (void*)dev_GetPixelShader;
    vt[90] = (void*)dev_DeletePixelShader;
    vt[91] = (void*)dev_SetPixelShaderConstant;
    vt[92] = (void*)dev_GetPixelShaderConstant;
    vt[93] = (void*)dev_GetPixelShaderFunction;
    vt[94] = (void*)dev_DrawRectPatch;
    vt[95] = (void*)dev_DrawTriPatch;
    vt[96] = (void*)dev_DeletePatch;
}
