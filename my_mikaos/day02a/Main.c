#include <Uifi.h>
#include <Library/UefiLib.h>

EFI_STATUS EFIAPI UsefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table) {
  Print(L"HEllo, Mikan World!\n");
  while(1);
  return EFI_SUCCESS;        
}
