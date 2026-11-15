#include <wincrypt.h>
static int supported_executable(void){
 wchar_t path[32768];
 if(!GetModuleFileNameW(NULL,path,32768))return 0;
 HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(f==INVALID_HANDLE_VALUE)return 0;
 HCRYPTPROV provider=0;HCRYPTHASH hash=0;int ok=0;DWORD size=32,read=0;BYTE buffer[65536],digest[32];char hex[65];
 if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))goto finish;
 if(!CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto finish;
 for(;;){if(!ReadFile(f,buffer,sizeof buffer,&read,NULL))goto finish;if(!read)break;if(!CryptHashData(hash,buffer,read,0))goto finish;}
 if(!CryptGetHashParam(hash,HP_HASHVAL,digest,&size,0))goto finish;
 for(int i=0;i<32;i++)snprintf(hex+i*2,3,"%02x",digest[i]);
 ok=!strcmp(hex,"920e149d3bb6acc0c2148e8814af0de2653952e95b4f03bd6b576adfff8d37d8")||!strcmp(hex,"d41e8e43280f743d8f8a8c39961f7e6e7abd84ad983aaf82401043d1fa9df233");
 logmsg("Executable SHA256 %s supported=%d",hex,ok);
 finish:
 if(hash)CryptDestroyHash(hash);
 if(provider)CryptReleaseContext(provider,0);
 CloseHandle(f);return ok;
}
static void stop_release(const char*message){
 logmsg("Stopped: %s",message);
#ifndef AUTOMATED_MATCH_TEST
 MessageBoxA(NULL,message,"Rivals Eight Local",MB_OK|MB_ICONERROR);
#endif
 ExitProcess(42);
}
