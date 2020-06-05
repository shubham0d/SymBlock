/*++

Module Name:

    testfilter.c

Abstract:

    This is the main module of the testfilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PFLT_FILTER gFilterHandle = NULL;
ULONG_PTR OperationStatusCtx = 1;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002



ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
testfilterInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
testfilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
testfilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
testfilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
testfilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
testfilterPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

VOID
testfilterOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    );

FLT_POSTOP_CALLBACK_STATUS
testfilterPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
testfilterFSPreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
testfilterFSPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
testfilterPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

BOOLEAN
testfilterDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    );

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, testfilterUnload)
#pragma alloc_text(PAGE, testfilterInstanceQueryTeardown)
#pragma alloc_text(PAGE, testfilterInstanceSetup)
#pragma alloc_text(PAGE, testfilterInstanceTeardownStart)
#pragma alloc_text(PAGE, testfilterInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {


    { IRP_MJ_FILE_SYSTEM_CONTROL,
      0,
      testfilterFSPreOperation,
      NULL },


    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    testfilterUnload,                           //  MiniFilterUnload

    NULL,                    //  InstanceSetup - testfilterInstanceSetup
    NULL,            //  InstanceQueryTeardown - testfilterInstanceQueryTeardown
    NULL,            //  InstanceTeardownStart - testfilterInstanceTeardownStart
    NULL,         //  InstanceTeardownComplete - testfilterInstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("testfilter!DriverEntry: Entered\n") );

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //

        status = FltStartFiltering( gFilterHandle );

        if (!NT_SUCCESS( status )) {

            FltUnregisterFilter( gFilterHandle );
        }
    }

    return status;
}

NTSTATUS
testfilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("testfilter!testfilterUnload: Entered\n") );

    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/



FLT_PREOP_CALLBACK_STATUS
testfilterFSPreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    NTSTATUS status;
    //ULONG inBuffer;
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    WCHAR NameOfTarget[310] = { 0 };
    //WCHAR MinorFunc[300] = { 0 };
    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("testfilter!testfilterPreOperation: Entered\n"));

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
    //FltObjects->FileObject->FileName;
    //Data->Iopb->TargetFileObject->FileName
    //Data->TagData->MountPointReparseBuffer.PathBuffer
    //FltObjects->FileObject->IrpList

    if (NT_SUCCESS(status))
    {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status))
        {
            
            // this is our target
            if (Data->Iopb->MinorFunction == IRP_MN_USER_FS_REQUEST) {

                // If Target filename is null
                if (Data->Iopb->TargetFileObject->FileName.Length == 0) {
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
                if (Data->Iopb->Parameters.DeviceIoControl.Buffered.IoControlCode != 589988) {
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
                
                if (Data->Iopb->TargetFileObject->FileName.Length < 300){
                    RtlCopyMemory(NameOfTarget, Data->Iopb->TargetFileObject->FileName.Buffer, Data->Iopb->TargetFileObject->FileName.Length);
                }
                else {
                    KdPrint(("Filename not saved since to large"));
                }

                if (Data->Iopb->Parameters.DeviceIoControl.Buffered.InputBufferLength > 0 && Data->Iopb->Parameters.DeviceIoControl.Buffered.OutputBufferLength == 0) {
                    REPARSE_DATA_BUFFER* inBuffer;
                    inBuffer = (REPARSE_DATA_BUFFER*)Data->Iopb->Parameters.DeviceIoControl.Buffered.SystemBuffer;




                    if (wcsstr(inBuffer->MountPointReparseBuffer.PathBuffer, L"\\RPC Control") != NULL) {
                        KdPrint(("Symlink Attack detected \r\n"));
                        KdPrint(("Directory Juntion from %ws to \\RPC Control \r\n",NameOfTarget));
                        Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
                        Data->IoStatus.Information = 0;
                        FltReleaseFileNameInformation(FileNameInfo);
                        return FLT_PREOP_COMPLETE;
                    }
                    if (wcsstr(inBuffer->MountPointReparseBuffer.PathBuffer, L"\\BaseNamedObjects") != NULL) {
                        KdPrint(("Symlink Attack detected \r\n"));
                        KdPrint(("Directory Juntion from %ws to \\BaseNamedObjects \r\n", NameOfTarget));
                        Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
                        Data->IoStatus.Information = 0;
                        FltReleaseFileNameInformation(FileNameInfo);
                        return FLT_PREOP_COMPLETE;
                    }
                    
                }
            }
            
        }
        
        

        FltReleaseFileNameInformation(FileNameInfo);
    }

        
  
    return FLT_PREOP_COMPLETE;
}


FLT_POSTOP_CALLBACK_STATUS
testfilterFSPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)


{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("testfilter!testfilterPostOperation: Entered\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}

