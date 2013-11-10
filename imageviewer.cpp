extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include "libavutil/common.h"
#include "libavdevice/avdevice.h"
}

#include <iostream>
#include <unistd.h>
#include <SDL/SDL.h>



using namespace std;
int main(int argc, char *argv[]){
cout <<"FFMPEG Image Viewer"<<endl;
cout <<"File Name:"<<argv[1]<<endl;
AVCodecContext *avctx;
AVFrame *frame = avcodec_alloc_frame();

/// Read Raw Binary Data of Image into an array 

FILE *f;
int64_t len;
int decode_ok;
f = fopen(argv[1],"rb");
fseek(f, 0, SEEK_END);
    len = ftell(f);
fseek(f, 0, SEEK_SET);
uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * len);
fread(data,len,1,f);
fclose(f);
AVPacket packet;
av_init_packet(&packet);
cout <<hex<<(int)data[0]<<endl;
cout <<hex<<(int)data[1]<<endl;
cout <<hex<<(int)data[2]<<endl;
cout <<hex<<(int)data[3]<<endl;
//sleep(1);

////////////////////////////////////////////////////////////////////////////////
/// Detect Image Format using FFMPEG funcrions

av_register_all();
AVFormatContext *pFormatCtx = avformat_alloc_context();
int videostream = -1;
int vs = 0;
avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
avformat_find_stream_info(pFormatCtx,NULL);

AVCodec *codec;
int i;
for(i=0; i<pFormatCtx->nb_streams; i++){
  switch(pFormatCtx->streams[i]->codec->codec_type){
case AVMEDIA_TYPE_VIDEO:
if(vs == 0){
videostream=i;
codec=avcodec_find_decoder(pFormatCtx->streams[i]->codec->codec_id);
}

vs = vs + 1;
break;
  }
    
}
////////////////////////////////////////////////////////////////////////////////

///Put Extraxted Raw Image data to ffmpeg's avpakect strcture

packet.data = data;
packet.size = len;
packet.flags = AV_PKT_FLAG_KEY;


  avctx = avcodec_alloc_context3(NULL);

/// Open codec of detected image format

    avcodec_register_all();
 avcodec_open2(avctx,codec, NULL);
   // avcodec_open2(avctx, avcodec_find_decoder(AV_CODEC_ID_MJPEG), NULL);

/// Decode Image
while(true){
  avcodec_decode_video2(avctx, frame, &decode_ok, &packet);
if(decode_ok){
break;
}
cout <<"Decoding..."<<endl;
}
cout <<"Height:"<<frame->height<<" , Width:"<<frame->width<<endl;

 /// Init SDL Library
 if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
    {
        printf("cannot initialize SDL\n");
        return EXIT_FAILURE;
    }
SDL_Surface *screen,*image;

/// Get Screen Size
const SDL_VideoInfo* info = SDL_GetVideoInfo();
int screen_width = info->current_w;
int screen_height = info->current_h;
int height;
int width;
cout <<"Screen Width:"<<screen_width<<endl;
cout <<"Screen Height:"<<screen_height<<endl;

///////////////////////////////////////////////////////////////////////////////
/// If image size is greater then screen size then reduce both width and height according to the original ratio
if(frame->width > screen_width || frame->height > screen_height){
int imgheight;
int imgwidth;
int x = 1;
while(true){
imgheight = frame->height / x;
imgwidth =  frame->width / x;
if(imgheight < 1024 && imgwidth < 768){
	break;
}

x = x + 1;
}
height = imgheight;
width = imgwidth;

}else{
height = frame->height;
width = frame->width;
}
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

//// convert pixel format to BGRA and rescale image using sws library
int numBytes;
uint8_t *vidbuffer;
AVFrame *frame1;
SwsContext * convert_ctx;
frame1 = avcodec_alloc_frame();

numBytes=avpicture_get_size(AV_PIX_FMT_BGRA ,width,height);
vidbuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
avpicture_fill((AVPicture *)frame1, vidbuffer, AV_PIX_FMT_BGRA,width,height);


convert_ctx = sws_getContext(frame->width, frame->height, avctx->pix_fmt,
                                   width, height,AV_PIX_FMT_BGRA,
                                   SWS_BICUBIC, NULL, NULL, NULL );

sws_scale(convert_ctx,frame->data,frame->linesize,0,frame->height,frame1->data,frame1->linesize);

////////////////////////////////////////////////////////////////


/// Create Window
int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | SDL_HWACCEL;
SDL_WM_SetCaption( "Simple FFMPEG based Image Viewer", NULL );

screen = SDL_SetVideoMode(width, height, 32, options);    

/// Create new SDL surface and point our decoded image buffer to that SDL surface
image = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, 0, 0, 0, 0);
image->pixels = frame1->data[0];
image->pitch = frame1->linesize[0];
  
/// Display our image
SDL_Event event;
SDL_Rect rect;
rect.x = 0;
rect.y = 0;
rect.w = width;
rect.h = height;
SDL_BlitSurface(image, NULL, screen, &rect);
SDL_Flip(screen);


/// Loop until program exits
while(true){
while(SDL_PollEvent(&event)){ 
switch(event.type){
case SDL_QUIT: 
return 0;
break;
}
break;
}
  SDL_Delay(1);
}
return 0;
}
