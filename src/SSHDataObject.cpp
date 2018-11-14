#include "SSHOperation.h"
#include "SSHDataObject.h"


extern LIBSSH::SFTPSession session;

STDMETHODIMP CDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr = p_->GetData(pformatetcIn, pmedium);
    return hr;
}

IFACEMETHODIMP CDataObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium )
{
    return p_->GetDataHere(pformatetc, pmedium);
}

IFACEMETHODIMP CDataObject::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    return p_->SetData(pformatetc, pmedium, fRelease);

}

STDMETHODIMP CDataObject::QueryGetData(FORMATETC *pformatetc)
{
    return p_->QueryGetData(pformatetc);
}

IFACEMETHODIMP CDataObject::GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut)
{
    return p_->GetCanonicalFormatEtc(pformatetcIn, pFormatetcOut);
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    return p_->EnumFormatEtc(dwDirection, ppenumFormatEtc);
}

IFACEMETHODIMP CDataObject::DAdvise(FORMATETC* pformatetc , DWORD  advf, IAdviseSink* pAdvSnk, DWORD* pdwConnection)
{
    return p_->DAdvise(pformatetc, advf, pAdvSnk, pdwConnection);
}

IFACEMETHODIMP CDataObject::DUnadvise(DWORD  dwConnection )
{
    return p_->DUnadvise(dwConnection);
}


IFACEMETHODIMP CDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
    return p_->EnumDAdvise(ppenumAdvise);
}

HRESULT CreateDataObjectInstance(LPCWSTR path,REFIID riid, void **ppv)
{
	*ppv = NULL;
	CDataObject *p = new (std::nothrow) CDataObject(path);
	HRESULT hr = p ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = p->QueryInterface(riid, ppv);
		p->Release();
	}
	return hr;
}
