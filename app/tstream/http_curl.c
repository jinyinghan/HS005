#include "http_curl.h"
#include <sys/stat.h>
#include "common_env.h"
#include "common_func.h"
#include <fcntl.h>

#include "log.h"

const char url_JsonHead[] = "Content-type:application/json;charset='utf-8'";
const char url_DefaultHead[] = "application/x-www-form-urlencoded";
const char Image_Head[] = "Content-type:image/jpeg";
const char Video_Head[] = "Content-type:video/mp4";
const char Event_Level[] = "Event-Level:1";
const char uploadType[] = "&uploadType=multipart";

size_t writefun (char *data, size_t size, size_t nmemb, void *stream)
{
	
	OutMemoryStruct *ms =(OutMemoryStruct*)stream;
	//if(ms->cur >= ms->size)
		//return 0;
	
	size_t getSize = size * nmemb;

	if(getSize > ms->size -ms->cur)
		getSize = ms->size -ms->cur;
	 
	//memcpy(ms->out+ms->cur, data, getSize);
	//ms->cur += getSize;
	memcpy(ms->out, data, getSize);
	return getSize;
}
size_t writenull (char *data, size_t size, size_t nmemb, void *stream)
{
	long getSize = size * nmemb;
	
	return 0;
	//return getSize;
}

size_t download_process(void *buffer,size_t size,size_t nmemb,void *user)
{
	size_t return_size = size*nmemb;
	FILE *fp = (FILE *)user;
	return_size = fwrite(buffer,size,nmemb,fp);
	printf("write %d\n",return_size);
	return return_size;	
}

int download_progress(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int *ret = (int *)clientp;
	*ret = (int)(100*dlnow/dltotal);
	if((*ret)%100 == 0)
//		printf("progress : %d",*ret);
//	上报百分比，一秒一次，只有0 / 100。
	return 0;
}

int http_download_file(char *url, int *progress, FILE *fp,  int verbose)
{
        CURL *handle;
        CURLcode ret = CURLE_OK;
        struct curl_slist * slist_urlHttpHead =  NULL;
        unsigned short second = 60*60; // 1 hour

        LOG("URL:%s\n", url);

        slist_urlHttpHead = curl_slist_append(slist_urlHttpHead, url_JsonHead);

        //Initialize the globle env
        ret = curl_global_init(CURL_GLOBAL_ALL);
        //    curl_dbg(ret);

        //get the handle of curl
        handle = curl_easy_init();
        if (handle) {
                // the attribute for curl
                ret = curl_easy_setopt(handle, CURLOPT_URL, url);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_TIMEOUT, second);
                curl_dbg(ret);

                //		curl_easy_setopt (handle, CURLOPT_FOLLOWLOCATION, 1); //页面重定向
                //        curl_dbg(ret);

                if (verbose) {
                        ret = curl_easy_setopt (handle, CURLOPT_VERBOSE, 1L);
                        curl_dbg(ret);
                }

                ret = curl_easy_setopt (handle, CURLOPT_HTTPHEADER, slist_urlHttpHead);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &download_process);
                curl_dbg(ret);

//                ret = curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
 //               curl_dbg(ret);

                if(progress)
                {
                        /*download progress configure*/
                        ret = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
                        curl_dbg(ret);

                        ret = curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, &download_progress);
                        curl_dbg(ret);

                        ret = curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, progress);
                        curl_dbg(ret);

                       // create_report_ota_progress_pthread();
                }

                ret = curl_easy_perform(handle);
                if(ret == CURLE_OK)
                        printf("curl perform success [%d]!\n",ret);
                else
                        curl_dbg(ret);

                long retcode = 0;
                CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
                if ((code == CURLE_OK) && (retcode == 200 )) {
                        ret = 0;
                } else {
                        if(retcode == 0)	//timeout again
                                ret = -1;
                        else
                                ret = 0;
                LOG("retcode %ld, ret %d, code %d, %s\n", retcode, ret, code, curl_easy_strerror(code));
                }
                curl_easy_cleanup(handle);
        }
        curl_global_cleanup();
        curl_slist_free_all(slist_urlHttpHead);	
        return ret;
}


int http_request (char *url_URL, char *url_Post, uint8_t certFlag,void *out,int verbose)
{
	CURL *handle;
	CURLcode ret = CURLE_OK;
	struct curl_slist * slist_urlHttpHead =  NULL;
	uint16_t second = 30;

#ifdef HTTP_DEBUG_MSG
	//    printf ( "URL:%s\nPost:%s\n", url_URL, url_Post );
#endif

    EMGCY_LOG("URL:%s\nPost:%s\n", url_URL, url_Post);

	slist_urlHttpHead = curl_slist_append ( slist_urlHttpHead, url_DefaultHead );

	/* In windows, this will init the winsock stuff */


	//curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	printf("~~~~~~~~~~~~~~~~~http_request post  start\n");
	handle = curl_easy_init();
	if(handle) {

		//curl_easy_setopt ( curl, CURLOPT_RETURNTRANSFER, 1 );
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		ret = curl_easy_setopt ( handle, CURLOPT_NOSIGNAL,  (long)1);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		
		ret = curl_easy_setopt ( handle, CURLOPT_URL,  url_URL );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}

		ret = curl_easy_setopt ( handle, CURLOPT_CONNECTTIMEOUT, 10);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_TIMEOUT, second );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
#if 0
		ret = curl_easy_setopt ( handle, CURLOPT_SSLVERSION,1);
		if(ret != CURLE_OK)
			printf ( "curl_easy_setopt() failed: %s\n",
					curl_easy_strerror(ret));
#endif
		ret = curl_easy_setopt ( handle, CURLOPT_SSL_VERIFYPEER, 0 );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_SSL_VERIFYHOST, 0 );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		//curl_easy_setopt ( curl, CURLOPT_SSLCERTTYPE, "PEM" );

		/*
		   if(certFlag == 2)
		   ret = curl_easy_setopt(curl,CURLOPT_SSLCERT,OFFI_CERT_PATH);	//offical evvironment
		   else
		   ret = curl_easy_setopt(curl,CURLOPT_SSLCERT,TEST_CERT_PATH);   // dev environment

		   if(ret != CURLE_OK)
		   printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		   printf("certflag = %d\n",certFlag);
		   */

		/*For HTTP */
		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYPEER,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYHOST,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt ( handle, CURLOPT_HTTPHEADER, slist_urlHttpHead );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_POST, 1 );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		// Now specify the POST data
		curl_easy_setopt ( handle, CURLOPT_POSTFIELDS, url_Post );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEDATA, out );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEFUNCTION, writefun);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform ( handle );
		/* Check for errors */
		if(ret != CURLE_OK) {
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		}

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
		if ( (code == CURLE_OK) && (retcode == 200 )){
			ret = 0;
		} else {
			printf("retcode:%ld\n",retcode);
			ret = -1;
		}

		/* always cleanup */
		curl_easy_cleanup(handle);
	}
	//curl_global_cleanup();
	curl_slist_free_all ( slist_urlHttpHead );
	printf("~~~~~~~~~~~~~~~~~~~~~~~~http_request post end\n");
	return ret;
}


int https_post_request(char * url_URL, char *url_Post, uint8_t certFlag, void *out, int verbose)
{
	CURL *handle;
	CURLcode ret = CURLE_OK;
	struct curl_slist * slist_urlHttpHead =  NULL;
	uint16_t second = 30;

	LOG( "URL:%s\nPost:%s\n", url_URL, url_Post );
	char cacert[32]={0},cert[32]={0},key[32]={0};

	strcpy(cacert, DF_CACERT_PATH);
	if (access(CERT_PATH, F_OK) == 0 && access(KEY_PATH, F_OK) == 0) {
		strcpy(cert, CERT_PATH);
		strcpy(key, KEY_PATH);
	}
	else {
		strcpy(cert, DF_CERT_PATH);
		strcpy(key, DF_KEY_PATH);
	}
	
	slist_urlHttpHead = curl_slist_append (slist_urlHttpHead, url_JsonHead );
	/* In windows, this will init the winsock stuff */

	
	//curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	handle = curl_easy_init();
	if (handle) {

		//curl_easy_setopt ( curl, CURLOPT_RETURNTRANSFER, 1 );
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		ret = curl_easy_setopt ( handle, CURLOPT_NOSIGNAL,  (long)1);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		
		ret = curl_easy_setopt ( handle, CURLOPT_URL,  url_URL );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}

		ret = curl_easy_setopt ( handle, CURLOPT_CONNECTTIMEOUT, 50);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_TIMEOUT, 30 );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);//openssl编译时使用curl官网或者firefox导出的第三方根证书文件 ;
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle,CURLOPT_CAINFO,cacert);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt(handle,CURLOPT_SSLCERT,cert);	//offical evvironment

		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_SSLKEY,key);	//offical evvironment
		
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt ( handle, CURLOPT_HTTPHEADER, slist_urlHttpHead );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_POST, 1 );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		// Now specify the POST data
		curl_easy_setopt ( handle, CURLOPT_POSTFIELDS, url_Post );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEDATA, out );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEFUNCTION, writefun);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform ( handle );
		/* Check for errors */
		if(ret != CURLE_OK) {
			printf ( "[%s][%d] curl_easy_perform() failed: %d, %s\n",__FUNCTION__,__LINE__,
					ret, curl_easy_strerror(ret));
		}

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
		LOG("retcode: %d\n", retcode);
		LOG("code: %d\n", code);
		if (((code == CURLE_OK) && (retcode == 200)) || (retcode == 202)){
			ret = 0;
		} else {
			pr_dbg("failed, (%d, %d),(%s, %s)\n", code, retcode, curl_easy_strerror(code), curl_easy_strerror(retcode));
			ret = -1;
			LOG("result: %s\n",out);
		}

		/* always cleanup */
		curl_easy_cleanup(handle);
	}
	//curl_global_cleanup();
	curl_slist_free_all(slist_urlHttpHead);

	return ret;
}

int https_post_request2(char * url_URL, char *url_Post, void *out, int verbose)
{
	CURL *handle;
	CURLcode ret = CURLE_OK;
	struct curl_slist * slist_urlHttpHead =  NULL;
	uint16_t second = 30;
	char cacert[32]={0},cert[32]={0},key[32]={0};

	LOG( "URL:%s\nPost:%s\n", url_URL, url_Post);
 	
    slist_urlHttpHead = curl_slist_append (slist_urlHttpHead, url_JsonHead);
	
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	handle = curl_easy_init();
	if (handle) {

		//curl_easy_setopt ( curl, CURLOPT_RETURNTRANSFER, 1 );
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		ret = curl_easy_setopt ( handle, CURLOPT_URL,  url_URL );
        curl_dbg(ret);

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
            curl_dbg(ret);
		}

//		ret = curl_easy_setopt ( handle, CURLOPT_CONNECTTIMEOUT, 50);
//		if(ret != CURLE_OK)
//			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
//					curl_easy_strerror(ret));
//
//		ret = curl_easy_setopt ( handle, CURLOPT_TIMEOUT, 30 );
//		if(ret != CURLE_OK)
//			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
//					curl_easy_strerror(ret));

        ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYPEER,0);
        curl_dbg(ret);
        
        ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYHOST,0);
        curl_dbg(ret);

		ret = curl_easy_setopt ( handle, CURLOPT_HTTPHEADER, slist_urlHttpHead);
        curl_dbg(ret);

		ret = curl_easy_setopt ( handle, CURLOPT_POST, 1 );
        curl_dbg(ret);

		// Now specify the POST data
		curl_easy_setopt ( handle, CURLOPT_POSTFIELDS, url_Post);
        curl_dbg(ret);


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEDATA, out);
        curl_dbg(ret);

		ret = curl_easy_setopt ( handle, CURLOPT_WRITEFUNCTION, writefun);
        curl_dbg(ret);


		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform ( handle );
		/* Check for errors */
        curl_dbg(ret);

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
		printf("retcode: %ld\n", retcode);
		printf("code: %d\n", code);
		if (((code == CURLE_OK) && (retcode == 200)) || (retcode == 201)){
			ret = 0;
		} else {
			pr_dbg("failed, (%d, %d),(%s, %s)\n", code, retcode, curl_easy_strerror(code), curl_easy_strerror(retcode));
			ret = -1;
//			printf("result: %s\n",out);
		}

		/* always cleanup */
		curl_easy_cleanup(handle);
	}
	//curl_global_cleanup();
	curl_slist_free_all(slist_urlHttpHead);

	return ret;
}

int https_get_request (char *url_URL,uint8_t certFlag,void *out,int verbose)
{
	CURL *handle;
	CURLcode ret = CURLE_OK;
	struct curl_slist * slist_urlHttpHead =  NULL;
	uint16_t second = 30;

	LOG( "URL:%s\n", url_URL);
	char cacert[32]={0},cert[32]={0},key[32]={0};
	strcpy(cacert,DF_CACERT_PATH);
	if(access(CERT_PATH,F_OK) == 0 && access(KEY_PATH,F_OK) == 0)
	{
		strcpy(cert,  CERT_PATH);
		strcpy(key,   KEY_PATH);
	}else{
		strcpy(cert,  DF_CERT_PATH);
		strcpy(key,   DF_KEY_PATH);
	}


	slist_urlHttpHead = curl_slist_append ( slist_urlHttpHead, url_JsonHead );

	/* In windows, this will init the winsock stuff */
	//curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	handle = curl_easy_init();
	if(handle) {

		//curl_easy_setopt ( curl, CURLOPT_RETURNTRANSFER, 1 );
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		ret = curl_easy_setopt ( handle, CURLOPT_NOSIGNAL,  (long)1);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		
		ret = curl_easy_setopt ( handle, CURLOPT_URL,  url_URL );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}

		ret = curl_easy_setopt ( handle, CURLOPT_CONNECTTIMEOUT, 10);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_TIMEOUT, second );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
#if 0
		ret = curl_easy_setopt ( handle, CURLOPT_SSLVERSION,1);
		if(ret != CURLE_OK)
			printf ( "curl_easy_setopt() failed: %s\n",
					curl_easy_strerror(ret));
#endif
		ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);//openssl编译时使用curl官网或者firefox导出的第三方根证书文件 ;
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle,CURLOPT_CAINFO,cacert);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt(handle,CURLOPT_SSLCERT,cert);	//offical evvironment

		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_SSLKEY,key);	//offical evvironment

		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

#if 0
		/*For HTTP */
		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYPEER,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYHOST,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);
#endif
		ret = curl_easy_setopt ( handle, CURLOPT_HTTPHEADER, slist_urlHttpHead );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
#if 1
		ret = curl_easy_setopt ( handle, CURLOPT_WRITEDATA, out );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));


		ret = curl_easy_setopt ( handle, CURLOPT_WRITEFUNCTION, writefun);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
#endif
		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform ( handle );
		/* Check for errors */
		if(ret != CURLE_OK) {
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		}

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
		if ( (code == CURLE_OK) && (retcode == 200 )){
			ret = 0;
			LOG("retcode: %d\n",retcode);
		} else {
			if(retcode == 0)	//timeout again
				ret = -1;
			else
				ret = 0;
			pr_error("retcode %d, ret %d, code %d, %s, result: %s\n", retcode, ret, code,
					curl_easy_strerror(code), out);
		}

		/* always cleanup */
		curl_easy_cleanup(handle);
	}
	//curl_global_cleanup();
	curl_slist_free_all ( slist_urlHttpHead );

	return ret;
}

#if 0  //post表单上传方式
int http_upload(char *url_URL,char *file_path,int res_type,long timestamp,int8_t certFlag,char *out,int verbose)
{
	CURL *handle;
	CURLcode ret = CURLE_OK;
	struct curl_slist * slist_urlHttpHead =  NULL;
	uint16_t second = 30;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	static const char buf[] = "Expect:";
	char timestamp_str[64] = {0};

	LOG( "URL:%s\nfile_path:%s\n", url_URL, file_path );

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

#if 1
	/* Fill in the file upload field */
	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "uploadfile",
			CURLFORM_FILE,"/mnt/mdrecord.mp4",
			CURLFORM_CONTENTTYPE,"video/mp4",
			CURLFORM_END);

	/* Fill in the filename field */
	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "filename",
			CURLFORM_COPYCONTENTS,"mdrecord.mp4",
			CURLFORM_END);
#endif
#if 0
	/* Fill in the submit field too, even if this is rarely needed */
	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "submit",
			CURLFORM_COPYCONTENTS, "send",
			CURLFORM_END);
#endif

	if(res_type==IMAGE_JPEG)
	{
		slist_urlHttpHead = curl_slist_append ( slist_urlHttpHead, Image_Head);
	}
	else
	{
		slist_urlHttpHead = curl_slist_append ( slist_urlHttpHead, Video_Head);
	}

	curl_slist_append ( slist_urlHttpHead, Event_Level);
	snprintf(timestamp_str,sizeof(timestamp_str),"TimeStamp:%ld",timestamp);
	curl_slist_append ( slist_urlHttpHead,timestamp_str);

	/* get a curl handle */
	handle = curl_easy_init();
	if(handle) {

		ret = curl_easy_setopt ( handle, CURLOPT_URL,  url_URL );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}

		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, slist_urlHttpHead);

		curl_easy_setopt(handle, CURLOPT_HTTPPOST, formpost);

		ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);//openssl编译时使用curl官网或者firefox导出的第三方根证书文件 ;
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_CONNECTTIMEOUT, 10);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_TIMEOUT, second );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(handle,CURLOPT_CAINFO,CACERT_PATH);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt(handle,CURLOPT_SSLCERT,KEY_PATH);	//offical evvironment
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt ( handle, CURLOPT_WRITEDATA, out );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( handle, CURLOPT_WRITEFUNCTION, writefun);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform ( handle );
		/* Check for errors */
		if(ret != CURLE_OK) {
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		}

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE , &retcode);
		if ( (code == CURLE_OK) && (retcode == 202 )){
			ret = 0;
			LOG("retcode: %d\n",retcode);
		} else {
			ret = -1;
			LOG("result: %s\n",out);
		}

		/* always cleanup */
		curl_easy_cleanup(handle);
	}
	curl_global_cleanup();
	/* then cleanup the formpost chain */
	curl_formfree(formpost);
	curl_slist_free_all ( slist_urlHttpHead );

	return ret;
}
#endif
void print_link(struct curl_slist *head)
{
	struct curl_slist *tmp;

	tmp = head;

	pr_dbg("链表打印开始!!!\n");
	while(tmp != NULL)
	{
		pr_dbg("输入的值为:num = %s,地址为:addr = %d\n",tmp->data,tmp);
		tmp = tmp->next;
	}
	pr_dbg("链表打印结束!!!\n");
}

struct curl_slist *http_head_append(struct curl_slist *head, char *event_level, int res_type, long timestamp)
{
	char timestamp_str[64] = {0};
	if(event_level == NULL) {
		return NULL;
	}
	head = curl_slist_append(head, event_level);
	if(res_type == IMAGE_JPEG) {
		curl_slist_append(head, Image_Head);
	}
	else if(res_type == VIDEO_MP4) {
		curl_slist_append(head, Video_Head);
	}
	else{
		pr_info("res_type %d\n", res_type);
	}
	CLEAR(timestamp_str);
	snprintf(timestamp_str,sizeof(timestamp_str),"TimeStamp:%ld",timestamp);
	curl_slist_append(head,timestamp_str);
	curl_slist_append(head,"Type:motion_event");
	//	print_link(head);
	return head;
}

int free_head(struct curl_slist *head)
{
	if(head == NULL) {
		return -1;
	}
	else{
		curl_slist_free_all (head);
		return 0;
	}
}

int auth_file_path_update(char *cacert, char *cert, char *key)
{
	if((cacert == NULL) || (cert == NULL) || (key == NULL)) {
		return -1;
	}
	strcpy(cacert,DF_CACERT_PATH);
	if(access(CERT_PATH,F_OK) == 0 && access(KEY_PATH,F_OK) == 0)
	{
		strcpy(cert,  CERT_PATH);
		strcpy(key,   KEY_PATH);
	}else{
		strcpy(cert,  DF_CERT_PATH);
		strcpy(key,   DF_KEY_PATH);
	}
	return 0;
}

int get_verbose(void)
{
	if(access("/tmp/cloud_dbg",F_OK)==0){
		pr_dbg("in \n");
		return 1;
	}
	else{
		pr_dbg("in \n");
		return 0;
	}
}
#if 1
int multi_post(const char *ul_file1, const char *ul_file2, char *url_URL, struct curl_slist *head, char *out)
{
	CURL *curl;

	CURLM *multi_handle;
	int still_running;
	CURLcode Cc_ret = CURLE_OK;
	CURLMcode CMc_ret = CURLM_OK;
	CURLMsg *msg;

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	int i_ret = 0;
	char cacert[32]={0},cert[32]={0},key[32]={0};
	pr_dbg("file_name %s, %s\n", ul_file1, ul_file2);

	/* Fill in the file upload field. This makes libcurl load data from
	   the given file name when curl_easy_perform() is called. */
	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "file",
			CURLFORM_FILE, ul_file1,
			CURLFORM_CONTENTTYPE,"video/mp4",
			CURLFORM_END);

	//   curl_formadd(&formpost,
	//				&lastptr,
	//				CURLFORM_COPYNAME, "uploadfile",
	//				CURLFORM_FILE, "/tmp/",
	//				CURLFORM_CONTENTTYPE,"video/mp4",
	//				CURLFORM_END);
	//
	//  /* Fill in the filename field */
	//  curl_formadd(&formpost,
	//			   &lastptr,
	//			   CURLFORM_COPYNAME, "file",
	//			   CURLFORM_COPYCONTENTS, "mdrecord.mp4",
	//			   CURLFORM_END);

	//  /* Fill in the submit field too, even if this is rarely needed */
	//  curl_formadd(&formpost,
	//			   &lastptr,
	//			   CURLFORM_COPYNAME, "submit",
	//			   CURLFORM_COPYCONTENTS, "send",
	//			   CURLFORM_END);

	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "file",
			CURLFORM_FILE, ul_file2,
			CURLFORM_CONTENTTYPE,"image/jpeg",
			CURLFORM_END);

	//  curl_formadd(&formpost,
	//			   &lastptr,
	//			   CURLFORM_COPYNAME, "uploadfile",
	//			   CURLFORM_FILE, "/tmp/",
	//			   CURLFORM_CONTENTTYPE,"video/jpeg",
	//			   CURLFORM_END);

	//  /* Fill in the filename field */
	//  curl_formadd(&formpost,
	//			   &lastptr,
	//			   CURLFORM_COPYNAME, "file",
	//			   CURLFORM_COPYCONTENTS, "mdrecord.jpeg",
	//			   CURLFORM_END);

	//  /* Fill in the submit field too, even if this is rarely needed */
	//  curl_formadd(&formpost,
	//			   &lastptr,
	//			   CURLFORM_COPYNAME, "submit",
	//			   CURLFORM_COPYCONTENTS, "send",
	//			   CURLFORM_END);

	curl = curl_easy_init();
	multi_handle = curl_multi_init();
	i_ret = auth_file_path_update((char *)&cacert, (char *)&cert, (char *)&key);
	if(i_ret < 0){
		printf ( "[%s][%d] in\n",__FUNCTION__,__LINE__);
	}


	/* initialize custom header list (stating that Expect: 100-continue is not
	   wanted */
	   printf("multi post start\n");
	if(curl && multi_handle) {

		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, url_URL);
		if(get_verbose() == 1){//printf the dbg msg
			Cc_ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_dbg(Cc_ret);
		}
		Cc_ret = curl_easy_setopt (curl, CURLOPT_NOSIGNAL,  (long)1);
		if(Cc_ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(Cc_ret));
		
		Cc_ret = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_dbg(Cc_ret);

		Cc_ret = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		curl_dbg(Cc_ret);

		Cc_ret =  curl_easy_setopt(curl,CURLOPT_CAINFO,cacert);
		curl_dbg(Cc_ret);

		Cc_ret = curl_easy_setopt(curl,CURLOPT_SSLCERT,cert);  //offical evvironment
		curl_dbg(Cc_ret);

		Cc_ret = curl_easy_setopt(curl,CURLOPT_SSLKEY,key); //offical evvironment
		curl_dbg(Cc_ret);
		Cc_ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_dbg(Cc_ret);

		Cc_ret =  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_dbg(Cc_ret);
		/* write to this file */
		Cc_ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
		curl_dbg(Cc_ret);

		Cc_ret = curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writefun);
		curl_dbg(Cc_ret);

		//	print_link(head);
		Cc_ret = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
		curl_dbg(Cc_ret);
		Cc_ret = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_dbg(Cc_ret);

		CMc_ret = curl_multi_add_handle(multi_handle, curl);
		multi_dbg(CMc_ret);

		CMc_ret = curl_multi_perform(multi_handle, &still_running);
		multi_dbg(CMc_ret);
		pr_dbg("out %s \n",out);
	   	LOG_LZF("out %s \n",out);
		do {
			struct timeval timeout;
			int rc; /* select() return code */
			CURLMcode mc; /* curl_multi_fdset() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd = -1;

			long curl_timeo = -1;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			CMc_ret = curl_multi_timeout(multi_handle, &curl_timeo);
			multi_dbg(CMc_ret);
			if(curl_timeo >= 0) {
				timeout.tv_sec = curl_timeo / 1000;
				if(timeout.tv_sec > 1)
					timeout.tv_sec = 1;
				else
					timeout.tv_usec = (curl_timeo % 1000) * 1000;
			}

			/* get file descriptors from the transfers */
			mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

			if(mc != CURLM_OK) {
				fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
				break;
			}

			/* On success the value of maxfd is guaranteed to be >= -1. We call
			   select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
			   no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
			   to sleep 100ms, which is the minimum suggested value in the
			   curl_multi_fdset() doc. */

			if(maxfd == -1) {
#ifdef _WIN32
				Sleep(100);
				rc = 0;
#else
				/* Portable sleep for platforms other than Windows. */
				struct timeval wait = { 0, 200 * 1000 }; /* 100ms */
				rc = select(0, NULL, NULL, NULL, &wait);
#endif
			}
			else {
				/* Note that on some platforms 'timeout' may be modified by select().
				   If you need access to the original value save a copy beforehand. */
				rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
			}
			//LOG_LZF("process ...... \n");
			switch(rc) {
				case -1:
					/* select error */
					break;
				case 0:
				default:
					/* timeout or readable/writable sockets */
					//		pr_dbg("perform!\n");
					CMc_ret = curl_multi_perform(multi_handle, &still_running);
					multi_dbg(CMc_ret);
					//		pr_dbg("running: %d!\n", still_running);
					LOG_LZF("running: %d!\n", still_running);
					break;
			}
		} while(still_running);
		char *url;
		long retcode = 0;
		//int totalTime = 0;
		long totalTime = 0;
		int msgs_left = 0;
		msg = curl_multi_info_read(multi_handle, &msgs_left);
		if(msg->msg == CURLMSG_DONE) {
			//third argument MUST be a pointer to a long, a pointer to a char * or a pointer to a double
			Cc_ret = curl_easy_getinfo(curl,CURLINFO_TOTAL_TIME,&totalTime);
			if((CURLE_OK==Cc_ret) && totalTime)
				pr_info("totalTime %ld\n", totalTime);
			Cc_ret = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
			curl_dbg(Cc_ret);
			//	  fprintf(stderr, "R: %d - %s <%s>\n",
			//			  msg->data.result, curl_easy_strerror(msg->data.result), url);

			Cc_ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
			if ( (Cc_ret == CURLE_OK) && (retcode == 202 )){
				i_ret = 0;
				pr_info("retcode: %d\n",retcode);
			} else {
				i_ret = -1;
				pr_error("failed, (%d, %d),(%s, %s)\n", Cc_ret, retcode, curl_easy_strerror(Cc_ret), curl_easy_strerror(retcode));
			}

		}
		else {
			pr_error("CURLMsg (%d)\n", msg->msg);
		}
		pr_dbg("ret %d, %d, %d\n", msg->msg, Cc_ret, retcode);
		LOG_LZF("ret %d, %d, %d\n", msg->msg, Cc_ret, retcode);
		//	pr_dbg("msg %d, %s, %s, %d", msg->msg, msg->easy_handle, msg->data.whatever, msg->data.result);
		CMc_ret = curl_multi_cleanup(multi_handle);
		multi_dbg(CMc_ret);

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		
	}
		if(formpost)
		curl_formfree(formpost);
		
		printf("multi post end\n");
	return i_ret;
}


int http_multi_upload(char *url_URL, int res_type, long timestamp, char *out)
{
	struct curl_slist * slist_urlHttpHead =  NULL;
	int transfers = 4;
	int i_ret = 0;
	if((url_URL == NULL) || (out == NULL))
		return -1;
	pr_dbg("url %s\n", url_URL);
	sprintf((char *)(url_URL+strlen(url_URL)),"%s",uploadType);
	pr_dbg("url %s\n", url_URL);
	slist_urlHttpHead = http_head_append(slist_urlHttpHead, (char *)Event_Level, res_type, timestamp);
	i_ret = multi_post(FILE_VIDEO, FILE_JPEG, url_URL, slist_urlHttpHead, out);
	if(i_ret < 0){
		pr_error("in\n");
	}
	free_head(slist_urlHttpHead);
	return i_ret;
}

#else
int http_upload(char *url_URL,char *buff,int size,int res_type,long timestamp,int8_t certFlag,char *out,int verbose)
{
	struct curl_slist * slist_urlHttpHead =  NULL;
	CURL *curl;
	char timestamp_str[64] = {0};
	uint16_t second = 30;
	int ret;

	if(buff==NULL)
		return -1;

	LOG( "res_type = %d , URL:%s\n",res_type, url_URL);
	char cacert[32]={0},cert[32]={0},key[32]={0};
	auth_file_path_update(&cacert, &cert, &key);
	pr_dbg("path %s, %s, %s\n", cacert, cert, key);
	//
	curl_global_init(CURL_GLOBAL_ALL);

	slist_urlHttpHead = http_head_append(slist_urlHttpHead, Event_Level, res_type, timestamp);
	print_link(slist_urlHttpHead);
	//
	curl = curl_easy_init();
	if (curl)
	{

		ret = curl_easy_setopt ( curl, CURLOPT_URL,  url_URL );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		if(verbose){//printf the dbg msg
			ret = curl_easy_setopt ( curl, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist_urlHttpHead);


		ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( curl, CURLOPT_CONNECTTIMEOUT, 10);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( curl, CURLOPT_TIMEOUT, second );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret =  curl_easy_setopt(curl,CURLOPT_CAINFO,cacert);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt(curl,CURLOPT_SSLCERT,cert);	//offical evvironment

		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(curl,CURLOPT_SSLKEY,key); //offical evvironment

		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, out );
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		ret = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, writefun);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));

		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);//指定大小，否则遇到'\0'就停止了
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buff);

		/* Perform the request, ret will get the return code */
		ret = curl_easy_perform (curl);
		/* Check for errors */
		if(ret != CURLE_OK) {
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		}

		long retcode = 0;
		CURLcode code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
		if ( (code == CURLE_OK) && (retcode == 202 )){
			ret = 0;
			LOG("retcode: %d\n",retcode);
		} else {
			ret = -1;
			LOG("result: %s\n",out);
		}

		/* always cleanup */
		curl_easy_cleanup(curl);


	}

	curl_global_cleanup();
	curl_slist_free_all ( slist_urlHttpHead );

	return ret;
}
#endif

int progress(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int *ret = (int *)clientp;
	*ret = (int)(100*dlnow/dltotal);
	if((*ret)%10 == 0)
		printf("progress : %d\n",*ret);
	return 0;
}

int http_download(char *url,int *process_data1 , int verbose)
{
	CURLcode ret;

	//Initialize the globle env
	CURL *handle = NULL;
	#if 0
	ret = curl_global_init(CURL_GLOBAL_ALL);
	if(ret != CURLE_OK)
	{
		printf("init libcurl failed !\n");
		return -1;
	}
	#endif
	int *process_data = process_data1;

	//get the handle of curl
	handle = curl_easy_init();
	printf("the url is ---%s--size=%d--\n",url,strlen(url));

	if(handle)
	{
		ret = curl_easy_setopt ( handle, CURLOPT_NOSIGNAL,  (long)1);
		if(ret != CURLE_OK)
			printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
					curl_easy_strerror(ret));
		// the attribute for curl
		ret = curl_easy_setopt(handle , CURLOPT_URL , url);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_TIMEOUT,120);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		curl_easy_setopt ( handle, CURLOPT_FOLLOWLOCATION, 1 );

		if(verbose){
			ret = curl_easy_setopt ( handle, CURLOPT_VERBOSE,  1L );
			if(ret != CURLE_OK)
				printf ( "[%s][%d] curl_easy_setopt() failed: %s\n",__FUNCTION__,__LINE__,
						curl_easy_strerror(ret));
		}
#if 0
		//set the json
		plist = curl_slist_append(plist,"Content-type:application/json;charset='utf-8'");
		ret = curl_easy_setopt(handle,CURLOPT_HTTPHEADER,plist);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);
#endif

		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYPEER,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_SSL_VERIFYHOST,0);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		FILE *fp = fopen("fw.bin","w+");
		ret = curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,&download_process);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_WRITEDATA,fp);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		/*download progress configure*/
		ret = curl_easy_setopt(handle,CURLOPT_NOPROGRESS,0L);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_PROGRESSFUNCTION,&progress);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_setopt(handle,CURLOPT_PROGRESSDATA,process_data);
		if(ret != CURLE_OK)
			printf("[%s][%d] set opt error ...\n",__FUNCTION__,__LINE__);

		ret = curl_easy_perform(handle);
		if(ret == CURLE_OK)
			printf("curl perform success [%d]!\n",ret);
		else
			printf("curl perform error [%s]\n",curl_easy_strerror(ret));

		fclose(fp);

		curl_easy_cleanup(handle);
	}
	//curl_global_cleanup();
	return ret;
}


