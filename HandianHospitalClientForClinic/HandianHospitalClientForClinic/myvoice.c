#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <process.h>

#include "msp_cmn.h"
#include "msp_errors.h"
#include "speech_recognizer.h"

#include "myvoice.h"

#define	BUFFER_SIZE	4096
#define LOG_SIZE (260)

static struct speech_rec iat;
static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

char ** m_pResult = NULL;

void on_result(const char *result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		//show_result(g_result, is_last);

		printf("Result: [ %s ]\n", g_result);

		*m_pResult = g_result;
	}
}
void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	printf("Start Listening...\n");
}
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
	{
		printf("\nSpeaking done \n");
	}		
	else
	{
		printf("\nRecognizer error %d\n", reason);
	}
}

int InitVoice()
{
	int			ret = MSP_SUCCESS;
	int			upload_on = 1; //是否上传用户词表
	int			mic_on = 1; //是否开始mic识别
	int			exit_on = 1; //是否结束
	const char* login_params = "appid = 59969e54, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
	int aud_src = 0;

	/*
	* sub:				请求业务类型
	* domain:			领域
	* language:			语言
	* accent:			方言
	* sample_rate:		音频采样率
	* result_type:		识别结果格式
	* result_encoding:	结果编码格式
	*
	*/
	const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	
	if (MSP_SUCCESS != ret) {
		printf("MSPLogin failed , Error code %d.\n", ret);
		return -1; //登录失败，退出登录
	}


	int errcode;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, DEFAULT_INPUT_DEVID, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return -1;
	}

	printf("InitVoice successed!\n");

	return 0;
}


/* demo recognize the audio from microphone */
void RecognizeVoice(char **pResult)
{
	int errcode;

	m_pResult = pResult;

	if (errcode = sr_start_listening(&iat)) {
		printf("start listen failed %d\n", errcode);
	}
	else {
		printf("start listen successed\n");
	}
}

/* demo recognize the audio from microphone */
void UninitVoice()
{
	sr_uninit(&iat);

	MSPLogout(); //退出登录

	printf("UninitVoice finished\n");
}
