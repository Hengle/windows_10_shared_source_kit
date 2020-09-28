/*++

Copyright (C) Microsoft. All rights reserved.

Module Name:

    classext.c

Abstract:

    This module implements the WDF class extension routines for the GPIO class
    extension driver.


Environment:

    Kernel mode

--*/

//
// ------------------------------------------------------------------- Includes
//

#include "pch.h"
#include <wdfcx.h>
#include <wdfldr.h>
#include "client.h"

#if defined(EVENT_TRACING)
#include "classext.tmh"                 // auto-generated by WPP
#endif

//
// -------------------------------------------------------------------- Defines
//

//
// Define the name for the GPIO class extension driver.
//

#define GPIO_CLASS_EXTENSION_NAME L"\\Device\\MSGpioClassExt"

//
// Define the minimum number of handlers that must be exported to a client.
//

#define GPIO_CLX_MINIMUM_EXPORTS (4)

// C_ASSERT(GpioExportLastExportIndex + 1 == 4)

//
// Maximum client driver's minor version that this extension can service
//

#define GPIO_CLX_CLIENT_MINOR_VERSION_MAX 0

//
// ----------------------------------------------------------------- Prototypes
//

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
GpioClxClassLibraryInitialize (
    VOID
    );

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
GpioClxClassLibraryDeinitialize (
    VOID
    );

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
GpioClxClassLibraryBindClient (
    __inout PWDF_CLASS_BIND_INFO ClassInformation,
    __in PWDF_COMPONENT_GLOBALS ClientGlobals
    );

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
GpioClxClassLibraryUnbindClient (
    __in PWDF_CLASS_BIND_INFO ClassInformation,
    __in PWDF_COMPONENT_GLOBALS ClientGlobals
    );

//
// -------------------------------------------------------------------- Globals
//

WDF_CLASS_LIBRARY_INFO GpioClxClassLibraryInformation =
{
    sizeof(WDF_CLASS_LIBRARY_INFO),         // Size
    {
        1,  // Major
        0,  // Minor
        0,  // Buid
    },                                      // Version

    GpioClxClassLibraryInitialize,         // ClassLibraryInitialize
    GpioClxClassLibraryDeinitialize,       // ClassLibraryDeinitialize
    GpioClxClassLibraryBindClient,         // ClassLibraryBindClient
    GpioClxClassLibraryUnbindClient,       // ClassLibraryUnbindClient
};

WDFDEVICE GpioClxClassLibraryDevice = NULL;

ULONG GpiopValidNumberOfExports[] =
{
    // Add number of exports of previous versions
    RTL_NUMBER_OF(GpioClxExports),  // Current version
};

//
// -------------------------------------------------------------------- Pragmas
//

#pragma alloc_text(PAGE, GpioClxClassLibraryInitialize)
#pragma alloc_text(PAGE, GpioClxClassLibraryDeinitialize)
#pragma alloc_text(PAGE, GpioClxClassLibraryBindClient)
#pragma alloc_text(PAGE, GpioClxClassLibraryUnbindClient)
#pragma alloc_text(PAGE, GpiopClassLibraryCreate)
#pragma alloc_text(PAGE, GpiopClassLibraryDestroy)

//
// --------------------------------------------------- Functions exposed to WDF
//

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
GpioClxClassLibraryInitialize (
    VOID
    )

/*++

Routine Description:

    This routine performs any initialization necessary at the time of class
    library creation. No action is required at present when the GPIO class
    extension is being created.

Arguments:

    None.

Return Value:

    NTSTATUS code.

--*/

{

    PAGED_CODE();

    return STATUS_SUCCESS;
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
GpioClxClassLibraryDeinitialize (
    VOID
    )

/*++

Routine Description:

    This routine performs any cleanup necessary at the time of class library
    destruction. No action is required at present when the GPIO class extension
    is being destroyed.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PAGED_CODE();

    return;
}

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
GpioClxClassLibraryBindClient (
    __inout PWDF_CLASS_BIND_INFO ClassInformation,
    __in PWDF_COMPONENT_GLOBALS ClientGlobals
    )

/*++

Routine Description:

    This routine is invoked by WDF when a client driver tries to bind to the
    class library. This routine initializes all the GPIO class extension handlers
    to be exported to client drivers.

Arguments:

    ClassInformation - Supplies a structure to be initialized with the exports.

    ClientGlobals - Supplies a pointer to the WDF_COMPONENT_GLOBALS structure
        allocated by WDF.

Return Value:

    NTSTATUS code.

--*/

{
    ULONG Count;
    ULONG Index;
    BOOLEAN Valid;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(ClientGlobals);

    //
    // The number of requested handlers must always match the number exported
    // from the GPIO class extension class library.
    //
    // N.B. It is possible that a down-level revision client could request a
    //      lower number of handlers. In such case, the number must be one of
    //      previous number of exports. Only those should be allowed to bind.
    //

    Count = 0;
    Valid = FALSE;
    for (Index = 0; Index < RTL_NUMBER_OF(GpiopValidNumberOfExports); Index++ ) {
        if (ClassInformation->FunctionTableCount ==
            GpiopValidNumberOfExports[Index]) {

            Count = GpiopValidNumberOfExports[Index];
            Valid = TRUE;
            break;
        }
    }

    if (Valid == FALSE) {
        TraceEvents(GpioLogHandle,
                    Error,
                    DBG_INIT,
                    "GpioClxClassLibraryBindClient: Incorrect function count! "
                    "Requested = %d\n",
                    ClassInformation->FunctionTableCount);

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Disallow newer client drivers binding to older client extensions.
    //

    if (ClassInformation->Version.Minor > GPIO_CLX_CLIENT_MINOR_VERSION_MAX) {
        TraceEvents(GpioLogHandle,
                    Error,
                    DBG_INIT,
                    "GpioClxClassLibraryBindClient: Unsupported minor version! "
                    "Requested = %d, Supported <= %d\n",
                    ClassInformation->Version.Minor,
                    GPIO_CLX_CLIENT_MINOR_VERSION_MAX);

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Supply the GPIO class extension's (partial) exports to the client
    // driver.
    //

    NT_ASSERT(Count <= RTL_NUMBER_OF(GpioClxExports));
    RtlCopyMemory(ClassInformation->FunctionTable,
                  &GpioClxExports[0],
                  sizeof(PGPIO_CLX_EXPORTED_INTERFACES) * Count);

    return STATUS_SUCCESS;
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
GpioClxClassLibraryUnbindClient (
    __in PWDF_CLASS_BIND_INFO ClassInformation,
    __in PWDF_COMPONENT_GLOBALS ClientGlobals
    )

/*++

Routine Description:

    This routine is invoked by WDF when a client driver tries to unbind. No
    action is required at present when a client unbinds.

Arguments:

    ClassInformation - Supplies a structure to be initialized with the exports.

    ClientGlobals - Supplies a pointer to the WDF_COMPONENT_GLOBALS structure
        allocated by WDF.

Return Value:

    None.

--*/

{

    PAGED_CODE();

    UNREFERENCED_PARAMETER(ClassInformation);
    UNREFERENCED_PARAMETER(ClientGlobals);
    return;
}

//
// ------------------------------------- Functions exposed to rest of the driver
//

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
GpiopClassLibraryCreate (
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This routine creates the WDF class library object for the GPIO class extension
    and registers it with driver framework. This is called when the GPIO class extension
    gets loaded (i.e., from DriverEntry).

Arguments:

    DriverObject - Pointer to the driver object created by the I/O manager.

    RegistryPath - Pointer to the driver specific registry key.

Return Value:

    NTSTATUS code.

--*/

{

    BOOLEAN DeviceCreated;
    PWDFDEVICE_INIT DeviceInit;
    DECLARE_UNICODE_STRING_SIZE(DriverName, 100);
    ULONG Index;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(DriverObject);

    DeviceCreated = FALSE;
    DeviceInit = WdfControlDeviceInitAllocate(WdfGetDriver(),
                                              &SDDL_DEVOBJ_KERNEL_ONLY);

    if (DeviceInit == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ClassLibraryCreateEnd;
    }

    Status = STATUS_OBJECT_NAME_COLLISION;
    for (Index = 0; Status == STATUS_OBJECT_NAME_COLLISION; Index += 1) {
        Status = RtlUnicodeStringPrintf(&DriverName,
                                        L"%s%d",
                                        GPIO_CLASS_EXTENSION_NAME,
                                        Index);

        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = WdfDeviceInitAssignName(DeviceInit, &DriverName);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = WdfDeviceCreate(&DeviceInit,
                                 WDF_NO_OBJECT_ATTRIBUTES,
                                 &GpioClxClassLibraryDevice);
    }

    if (!NT_SUCCESS(Status)) {
        goto ClassLibraryCreateEnd;
    }

    DeviceCreated = TRUE;
    WdfControlFinishInitializing(GpioClxClassLibraryDevice);

    //
    // Register the GPIO class extension as a class library with the driver
    // framework. This will allow client driver drivers to bind with the class
    // extension.
    //

    Status = WdfRegisterClassLibrary(&GpioClxClassLibraryInformation,
                                     RegistryPath,
                                     &DriverName);

ClassLibraryCreateEnd:
    if (!NT_SUCCESS(Status)) {
        if (GpioClxClassLibraryDevice != NULL) {
            WdfObjectDelete(GpioClxClassLibraryDevice);
            GpioClxClassLibraryDevice = NULL;
        }

        //
        // Free the WDFDEVICE_INIT structure only if the device creation fails.
        // Otherwise framework frees the memory itself.
        //

        if ((DeviceInit != NULL) && (DeviceCreated == FALSE)) {
            WdfDeviceInitFree(DeviceInit);
        }
    }

    return Status;
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
GpiopClassLibraryDestroy (
    __in PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This routine deletes the WDF class library object created for the GPIO
    class extension.

Arguments:

    DriverObject - Pointer to the driver object created by the I/O manager.

Return Value:

    None.

--*/

{

    PAGED_CODE();

    UNREFERENCED_PARAMETER(DriverObject);

    if (GpioClxClassLibraryDevice != NULL) {
        WdfObjectDelete(GpioClxClassLibraryDevice);
        GpioClxClassLibraryDevice = NULL;
    }

    return;
}

