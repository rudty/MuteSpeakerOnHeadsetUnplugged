#include <iostream>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <Windows.h>
#include <endpointvolume.h>
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }
#define EXIT_IF_FAILED(__RUN__) if(FAILED(__RUN__)) { \
									MessageBoxA(NULL, "RUN_FAILED", "mute", MB_OK);\
									exit(1); }
class CMMNotificationClient : public IMMNotificationClient
{
	
	IAudioEndpointVolume* defaultAudioVolume = nullptr;

public:
	CMMNotificationClient() { }

	void attachVolumeNotification() {
		/**
		 * 디바이스로부터 기본 volume조절기를 가져와서 등록
		 * (defaultAudioVolume)
		 * 나머지는 해제
		 */
		IMMDeviceEnumerator *deviceEnumerator = nullptr;
		EXIT_IF_FAILED(CoCreateInstance(
			__uuidof(MMDeviceEnumerator), 
			nullptr,
			CLSCTX_ALL, 
			__uuidof(IMMDeviceEnumerator),
			(void**)&deviceEnumerator));

		EXIT_IF_FAILED(deviceEnumerator->RegisterEndpointNotificationCallback(this));
		
		IMMDevice* device = nullptr;
		EXIT_IF_FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device));

		EXIT_IF_FAILED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&defaultAudioVolume));
		
		SAFE_RELEASE(device);
		SAFE_RELEASE(deviceEnumerator);
	}

	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId,DWORD dwNewState){
		/**
		 * 여기서 실제적으로 mute를 설정.
		 * device가 
		 */
		switch (dwNewState)
		{
		case DEVICE_STATE_ACTIVE:
#ifdef _DEBUG
			printf("DEVICE_STATE_ACTIVE\n");
#endif
			break;
		case DEVICE_STATE_UNPLUGGED:
			defaultAudioVolume->SetMute(true, nullptr);
#ifdef _DEBUG
			printf("DEVICE_STATE_UNPLUGGED\n");
#endif
			break;
		}
		return S_OK;
	}

	~CMMNotificationClient() {
		SAFE_RELEASE(defaultAudioVolume);
	}

	/**
	 * 이 아래부터는 COM 인터페이스라서 건들필요 없음
	 */
	ULONG STDMETHODCALLTYPE AddRef() { return 1; }

	ULONG STDMETHODCALLTYPE Release(){ return 1; }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface){
		return S_OK;
	}
	
	/**
	 * 이 아래부터는 사용하지 않는 IMM 이벤트이므로 그냥 S_OK 반환
	 */
	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId){
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) {
		return S_OK;
	};

	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) {
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId,	const PROPERTYKEY key){
		return S_OK;
	}
};


#ifdef _DEBUG
int main() {
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CMMNotificationClient c;
	c.attachVolumeNotification();

	getchar(); 
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CMMNotificationClient c;
	c.attachVolumeNotification();


	//좀비 프로세스처럼 동작할거라서 
	//계속 꺼지지않아도 상관없음
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0));
	return 0; 
}
#endif