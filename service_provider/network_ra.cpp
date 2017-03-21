/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "network_ra.h"
#include "service_provider.h"

//client.cpp
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
//#define SERVER_PORT 84
// #pragma pack(1)
// typedef struct _ra_samp_request_header_t{
//     uint8_t  type;     /* set to one of ra_msg_type_t*/
//     uint32_t size;     /*size of request body*/
//     uint8_t  align[3];
//     uint8_t body[];
// } ra_samp_request_header_t;

// typedef struct _ra_samp_response_header_t{
//     uint8_t  type;      /* set to one of ra_msg_type_t*/
//     uint8_t  status[2];
//     uint32_t size;      /*size of the response body*/
//     uint8_t  align[1];
//     uint8_t  body[];
// } ra_samp_response_header_t;
// #pragma pack()
// //------------------------

#define REV 1024*1024
#define SEND 512
#define RECEIVE 512

#define MSG0_REQ_SIZE sizeof(ra_samp_request_header_t)+sizeof(uint32_t)
#define MSG0_RESP_SIZE sizeof(ra_samp_response_header_t)+sizeof(uint32_t)

#define MSG1_REQ_SIZE sizeof(ra_samp_request_header_t)+sizeof(sgx_ra_msg1_t)
#define MSG1_RESP_SIZE sizeof(ra_samp_request_header_t)+sizeof(sgx_ra_msg1_t)

// #define MSG2_REQ_SIZE
// #define MSG2_RESP_SIZE

// #define MSG3_REQ_SIZE
// #define MSG3_RESP_SIZE

typedef unsigned char byte;
char* start_msg="_START_";

//@data :input message string
//@sockfd :sock handler
//return : -1 error
//       : 0 success    
int str_write(int sockfd,byte* data,int len)
{
    ssize_t ret =send(sockfd,data,len,0);
    if(ret == -1){
        printf("sending message error: %s\n",(char*)strerror(errno));
        return -1;
    }else if(ret >= 0){
        printf("send data bytes %ld\n",ret);
        return 0;
    }else{
        printf("unknow error!\n");  
    }
    return -1;
}

byte* str_read(int sockfd,int* len)
{
    ssize_t n;
    byte line[RECEIVE];
    byte* instream=(byte*)malloc(sizeof(char)*REV);
    printf("in read\n");
    if((n=recv(sockfd,line,RECEIVE,0))>0){
        printf("merge file...%d...\n",n);
        line[n]='\0';
        memcpy(instream,line,n);
        bzero(&line,sizeof(line));
        *len=n;
    }
        printf("read ok\n");
    
    return instream;
}
// Used to send requests to the service provider sample.  It
// simulates network communication between the ISV app and the
// ISV service provider.  This would be modified in a real
// product to use the proper IP communication.
//
// @param server_url String name of the server URL
// @param p_req Pointer to the message to be sent.
// @param p_resp Pointer to a pointer of the response message.

// @return int

int ra_network_send_receive(const char *server_url,
    const ra_samp_request_header_t *p_req,
    ra_samp_response_header_t **p_resp)
{
    int ret = 0;
    ra_samp_response_header_t* p_resp_msg;

    if((NULL == server_url) ||
        (NULL == p_req) ||
        (NULL == p_resp))
    {
        return -1;
    }

    switch(p_req->type)
    {

    case TYPE_RA_MSG0:
        ret = sp_ra_proc_msg0_req((const sample_ra_msg0_t*)((uint8_t*)p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size);
	//ret = sp_ra_proc_msg0_req();
        if (0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg1_req fail [%s].",
                __FUNCTION__);
        }
        break;

    case TYPE_RA_MSG1:
        ret = sp_ra_proc_msg1_req((const sample_ra_msg1_t*)((uint8_t*)p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        if(0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg1_req fail [%s].",
                __FUNCTION__);
        }
        else
        {
            *p_resp = p_resp_msg;
        }
        break;

    case TYPE_RA_MSG3:
        ret =sp_ra_proc_msg3_req((const sample_ra_msg3_t*)((uint8_t*)p_req +
            sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        if(0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg3_req fail [%s].",
                __FUNCTION__);
        }
        else
        {
            *p_resp = p_resp_msg;
        }
        break;

    default:
        ret = -1;
        fprintf(stderr, "\nError, unknown ra message type. Type = %d [%s].",
            p_req->type, __FUNCTION__);
        break;
    }

    return ret;
}

// Used to free the response messages.  In the sample code, the
// response messages are allocated by the SP code.
//
//
// @param resp Pointer to the response buffer to be freed.

void ra_free_network_response_buffer(ra_samp_response_header_t *resp)
{
    if(resp!=NULL)
    {
        free(resp);
    }
}



int main(int argc, char **argv)
{
    //--------------------prepare---------------------
    int sockfd;
    struct sockaddr_in servaddr;
    if(argc==1||argc==2){
        printf("error:please input (IP PORT)\n");
        return -1;
    }
    if(argc>4){
        printf("error:too many arguments!\n");
        return -1;
    }

    if(argc==3){
        printf("warning:you are sending an empty message to server\n");
    }

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        printf("socket established error: %s\n",(char*)strerror(errno)); return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));//set port
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);//set ip

    printf("client try to connect\n");
    int conRes=connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if(conRes==-1)
    {    
        printf("connect error: %s\n",strerror(errno)); return -1;
    }else{
        printf("connect success\n");    
    }
    //-----------------start-------------------
    char start[9];
    char* data;
    char* resp="OK";
    //receive start
    //int len=read(sockfd,start,9);
    int len=recv(sockfd,start,9,0);
    start[len]='\0';
    printf("msg:%s\n",start); 
    //response
    if(strcmp(start,start_msg)==0){
        printf("equal\n");
    }else{
        printf("not equal\n");
    }
    //-----change mode
    int optval=0;
    int rets ;
    // rets = setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(int));
    // rets = setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(int));
    
    //sleep(5);
    //ssize_t ret = write(sockfd,resp,strlen((char*)resp));
    int ret = send(sockfd,resp,strlen(resp),0);
    if(ret>=0){
        printf("send ok\n");
    }else{
        exit(0);
    }
    // char aa[100];
    // len = recv(sockfd,aa,100,0);
    // aa[len]='\0';
    // printf("aa=%s\n",aa);
    // send(sockfd,aa,100,0);
    int *read_len=(int*)malloc(sizeof(int));
    byte* in_data = str_read(sockfd,read_len);
    printf("readlength:%d\n",*read_len);
    ra_samp_request_header_t* p = (ra_samp_request_header_t*)malloc(*read_len);
    //printf("in_data%s\n",(char*)in_data );
    //yyb_form_request_header(p,in_data,12);
    memcpy(p,in_data,*read_len);
    //p=(ra_samp_request_header_t*)in_data;
    // for(int i=0;i<12;i++){
    //  printf("%d==%c\n",i,in_data[i]);
    // }
    // printf("2222\n");
    //p->body[3]='\0';
    printf("ra==%s,%s,%d\n",p->align,p->body,p->size);

    ra_samp_response_header_t* pp = (ra_samp_response_header_t*)malloc(sizeof(ra_samp_response_header_t)+4);
    memset(pp,0,sizeof(ra_samp_response_header_t)+4);
        pp->type='b';
        pp->size=4;
        char pppp[]="rsp";
        strcpy((char*)(pp->body),pppp);
    byte tmp[SEND];
    memcpy(tmp,pp,sizeof(ra_samp_response_header_t)+4);
    str_write(sockfd,tmp,sizeof(ra_samp_response_header_t)+4);
    
    close(sockfd);
    exit(0);
}
