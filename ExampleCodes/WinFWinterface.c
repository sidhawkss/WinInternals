/*
Copyright (c) Microsoft Corporation
SYNOPSIS
Sample code for the Windows Firewall COM interface.
*/
#include <windows.h>
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>
#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )
HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile)
{
HRESULT hr = S_OK;
INetFwMgr* fwMgr = NULL;
INetFwPolicy* fwPolicy = NULL;
_ASSERT(fwProfile != NULL);
*fwProfile = NULL;
// Create an instance of the firewall settings manager.
hr = CoCreateInstance(
__uuidof(NetFwMgr),
NULL,
CLSCTX_INPROC_SERVER,
__uuidof(INetFwMgr),
(void**)&fwMgr
);
if (FAILED(hr))
{
printf("CoCreateInstance failed: 0x%08lx\n", hr);
goto error;
}
// Retrieve the local firewall policy.
hr = fwMgr->get_LocalPolicy(&fwPolicy);
if (FAILED(hr))
{
printf("get_LocalPolicy failed: 0x%08lx\n", hr);
goto error;
}
// Retrieve the firewall profile currently in effect.
hr = fwPolicy->get_CurrentProfile(fwProfile);
if (FAILED(hr))
{
3/14
printf("get_CurrentProfile failed: 0x%08lx\n", hr);
goto error;
}
error:
// Release the local firewall policy.
if (fwPolicy != NULL)
{
fwPolicy->Release();
}
// Release the firewall settings manager.
if (fwMgr != NULL)
{
fwMgr->Release();
}
return hr;
}
void WindowsFirewallCleanup(IN INetFwProfile* fwProfile)
{
// Release the firewall profile.
if (fwProfile != NULL)
{
fwProfile->Release();
}
}
HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn)
{
HRESULT hr = S_OK;
VARIANT_BOOL fwEnabled;
_ASSERT(fwProfile != NULL);
_ASSERT(fwOn != NULL);
*fwOn = FALSE;
// Get the current state of the firewall.
hr = fwProfile->get_FirewallEnabled(&fwEnabled);
if (FAILED(hr))
{
printf("get_FirewallEnabled failed: 0x%08lx\n", hr);
goto error;
}
// Check to see if the firewall is on.
if (fwEnabled != VARIANT_FALSE)
{
*fwOn = TRUE;
printf("The firewall is on.\n");
4/14
}
else
{
printf("The firewall is off.\n");
}
error:
return hr;
}
HRESULT WindowsFirewallTurnOn(IN INetFwProfile* fwProfile)
{
HRESULT hr = S_OK;
BOOL fwOn;
_ASSERT(fwProfile != NULL);
// Check to see if the firewall is off.
hr = WindowsFirewallIsOn(fwProfile, &fwOn);
if (FAILED(hr))
{
printf("WindowsFirewallIsOn failed: 0x%08lx\n", hr);
goto error;
}
// If it is, turn it on.
if (!fwOn)
{
// Turn the firewall on.
hr = fwProfile->put_FirewallEnabled(VARIANT_TRUE);
if (FAILED(hr))
{
printf("put_FirewallEnabled failed: 0x%08lx\n", hr);
goto error;
}
printf("The firewall is now on.\n");
}
error:
return hr;
}
HRESULT WindowsFirewallTurnOff(IN INetFwProfile* fwProfile)
{
HRESULT hr = S_OK;
BOOL fwOn;
_ASSERT(fwProfile != NULL);
// Check to see if the firewall is on.
5/14
hr = WindowsFirewallIsOn(fwProfile, &fwOn);
if (FAILED(hr))
{
printf("WindowsFirewallIsOn failed: 0x%08lx\n", hr);
goto error;
}
// If it is, turn it off.
if (fwOn)
{
// Turn the firewall off.
hr = fwProfile->put_FirewallEnabled(VARIANT_FALSE);
if (FAILED(hr))
{
printf("put_FirewallEnabled failed: 0x%08lx\n", hr);
goto error;
}
printf("The firewall is now off.\n");
}
error:
return hr;
}
HRESULT WindowsFirewallAppIsEnabled(
IN INetFwProfile* fwProfile,
IN const wchar_t* fwProcessImageFileName,
OUT BOOL* fwAppEnabled
)
{
HRESULT hr = S_OK;
BSTR fwBstrProcessImageFileName = NULL;
VARIANT_BOOL fwEnabled;
INetFwAuthorizedApplication* fwApp = NULL;
INetFwAuthorizedApplications* fwApps = NULL;
_ASSERT(fwProfile != NULL);
_ASSERT(fwProcessImageFileName != NULL);
_ASSERT(fwAppEnabled != NULL);
*fwAppEnabled = FALSE;
// Retrieve the authorized application collection.
hr = fwProfile->get_AuthorizedApplications(&fwApps);
if (FAILED(hr))
{
printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
goto error;
}
// Allocate a BSTR for the process image file name.
fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
6/14
if (fwBstrProcessImageFileName == NULL)
{
hr = E_OUTOFMEMORY;
printf("SysAllocString failed: 0x%08lx\n", hr);
goto error;
}
// Attempt to retrieve the authorized application.
hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
if (SUCCEEDED(hr))
{
// Find out if the authorized application is enabled.
hr = fwApp->get_Enabled(&fwEnabled);
if (FAILED(hr))
{
printf("get_Enabled failed: 0x%08lx\n", hr);
goto error;
}
if (fwEnabled != VARIANT_FALSE)
{
// The authorized application is enabled.
*fwAppEnabled = TRUE;
printf(
"Authorized application %lS is enabled in the firewall.\n",
fwProcessImageFileName
);
}
else
{
printf(
"Authorized application %lS is disabled in the firewall.\n",
fwProcessImageFileName
);
}
}
else
{
// The authorized application was not in the collection.
hr = S_OK;
printf(
"Authorized application %lS is disabled in the firewall.\n",
fwProcessImageFileName
);
}
error:
// Free the BSTR.
SysFreeString(fwBstrProcessImageFileName);
// Release the authorized application instance.
if (fwApp != NULL)
7/14
{
fwApp->Release();
}
// Release the authorized application collection.
if (fwApps != NULL)
{
fwApps->Release();
}
return hr;
}
HRESULT WindowsFirewallAddApp(
IN INetFwProfile* fwProfile,
IN const wchar_t* fwProcessImageFileName,
IN const wchar_t* fwName
)
{
HRESULT hr = S_OK;
BOOL fwAppEnabled;
BSTR fwBstrName = NULL;
BSTR fwBstrProcessImageFileName = NULL;
INetFwAuthorizedApplication* fwApp = NULL;
INetFwAuthorizedApplications* fwApps = NULL;
_ASSERT(fwProfile != NULL);
_ASSERT(fwProcessImageFileName != NULL);
_ASSERT(fwName != NULL);
// First check to see if the application is already authorized.
hr = WindowsFirewallAppIsEnabled(
fwProfile,
fwProcessImageFileName,
&fwAppEnabled
);
if (FAILED(hr))
{
printf("WindowsFirewallAppIsEnabled failed: 0x%08lx\n", hr);
goto error;
}
// Only add the application if it isn't already authorized.
if (!fwAppEnabled)
{
// Retrieve the authorized application collection.
hr = fwProfile->get_AuthorizedApplications(&fwApps);
if (FAILED(hr))
{
printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
goto error;
}
// Create an instance of an authorized application.
8/14
hr = CoCreateInstance(
__uuidof(NetFwAuthorizedApplication),
NULL,
CLSCTX_INPROC_SERVER,
__uuidof(INetFwAuthorizedApplication),
(void**)&fwApp
);
if (FAILED(hr))
{
printf("CoCreateInstance failed: 0x%08lx\n", hr);
goto error;
}
// Allocate a BSTR for the process image file name.
fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
if (fwBstrProcessImageFileName == NULL)
{
hr = E_OUTOFMEMORY;
printf("SysAllocString failed: 0x%08lx\n", hr);
goto error;
}
// Set the process image file name.
hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
if (FAILED(hr))
{
printf("put_ProcessImageFileName failed: 0x%08lx\n", hr);
goto error;
}
// Allocate a BSTR for the application friendly name.
fwBstrName = SysAllocString(fwName);
if (SysStringLen(fwBstrName) == 0)
{
hr = E_OUTOFMEMORY;
printf("SysAllocString failed: 0x%08lx\n", hr);
goto error;
}
// Set the application friendly name.
hr = fwApp->put_Name(fwBstrName);
if (FAILED(hr))
{
printf("put_Name failed: 0x%08lx\n", hr);
goto error;
}
// Add the application to the collection.
hr = fwApps->Add(fwApp);
if (FAILED(hr))
{
printf("Add failed: 0x%08lx\n", hr);
goto error;
}
9/14
printf(
"Authorized application %lS is now enabled in the firewall.\n",
fwProcessImageFileName
);
}
error:
// Free the BSTRs.
SysFreeString(fwBstrName);
SysFreeString(fwBstrProcessImageFileName);
// Release the authorized application instance.
if (fwApp != NULL)
{
fwApp->Release();
}
// Release the authorized application collection.
if (fwApps != NULL)
{
fwApps->Release();
}
return hr;
}
HRESULT WindowsFirewallPortIsEnabled(
IN INetFwProfile* fwProfile,
IN LONG portNumber,
IN NET_FW_IP_PROTOCOL ipProtocol,
OUT BOOL* fwPortEnabled
)
{
HRESULT hr = S_OK;
VARIANT_BOOL fwEnabled;
INetFwOpenPort* fwOpenPort = NULL;
INetFwOpenPorts* fwOpenPorts = NULL;
_ASSERT(fwProfile != NULL);
_ASSERT(fwPortEnabled != NULL);
*fwPortEnabled = FALSE;
// Retrieve the globally open ports collection.
hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
if (FAILED(hr))
{
printf("get_GloballyOpenPorts failed: 0x%08lx\n", hr);
goto error;
}
// Attempt to retrieve the globally open port.
hr = fwOpenPorts->Item(portNumber, ipProtocol, &fwOpenPort);
10/14
if (SUCCEEDED(hr))
{
// Find out if the globally open port is enabled.
hr = fwOpenPort->get_Enabled(&fwEnabled);
if (FAILED(hr))
{
printf("get_Enabled failed: 0x%08lx\n", hr);
goto error;
}
if (fwEnabled != VARIANT_FALSE)
{
// The globally open port is enabled.
*fwPortEnabled = TRUE;
printf("Port %ld is open in the firewall.\n", portNumber);
}
else
{
printf("Port %ld is not open in the firewall.\n", portNumber);
}
}
else
{
// The globally open port was not in the collection.
hr = S_OK;
printf("Port %ld is not open in the firewall.\n", portNumber);
}
error:
// Release the globally open port.
if (fwOpenPort != NULL)
{
fwOpenPort->Release();
}
// Release the globally open ports collection.
if (fwOpenPorts != NULL)
{
fwOpenPorts->Release();
}
return hr;
}
HRESULT WindowsFirewallPortAdd(
IN INetFwProfile* fwProfile,
IN LONG portNumber,
IN NET_FW_IP_PROTOCOL ipProtocol,
IN const wchar_t* name
)
{

