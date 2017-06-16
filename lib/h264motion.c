#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/motion_vector.h>

#include <SDL2/SDL.h>




void ffmpeg_print_error(int err) // copied from cmdutils.c, originally called print_error
{
	char errbuf[128];
	const char *errbuf_ptr = errbuf;

	if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
		errbuf_ptr = strerror(AVUNERROR(err));
	av_log(NULL, AV_LOG_ERROR, "ffmpeg_print_error: %s\n", errbuf_ptr);
}


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void putpixel2(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	putpixel(surface, x*2, y*2, pixel);
	putpixel(surface, x*2 + 1, y*2, pixel);
	putpixel(surface, x*2, y*2 + 1, pixel);
	putpixel(surface, x*2 + 1, y*2 + 1, pixel);
}

void ffmpeg_init(char * filename)
{
	av_register_all();

	av_log_set_level(AV_LOG_QUIET);
	//av_log_set_callback(ffmpeg_log_callback_null);

	AVFrame * ffmpeg_pFrame = av_frame_alloc();
	AVFormatContext * ffmpeg_pFormatCtx = avformat_alloc_context();
	AVStream* ffmpeg_pVideoStream = NULL;
	int ffmpeg_videoStreamIndex = -1;
	size_t ffmpeg_frameWidth = 0;
	size_t ffmpeg_frameHeight = 0;

	int err = 0;

	if ((err = avformat_open_input(&ffmpeg_pFormatCtx, filename, NULL, NULL)) != 0)
	{
		ffmpeg_print_error(err);
	}

	if ((err = avformat_find_stream_info(ffmpeg_pFormatCtx, NULL)) < 0)
	{
		ffmpeg_print_error(err);
	}

	for(int i = 0; i < ffmpeg_pFormatCtx->nb_streams; i++)
	{
		AVCodecContext *enc = ffmpeg_pFormatCtx->streams[i]->codec;
		if( AVMEDIA_TYPE_VIDEO == enc->codec_type && ffmpeg_videoStreamIndex < 0 )
		{
			AVCodec *pCodec = avcodec_find_decoder(enc->codec_id);
			AVDictionary *opts = NULL;
			av_dict_set(&opts, "flags2", "+export_mvs", 0);
			if (!pCodec || avcodec_open2(enc, pCodec, &opts) < 0)
			{
				printf("Codec not found or cannot open codec.");
			}

			ffmpeg_videoStreamIndex = i;
			ffmpeg_pVideoStream = ffmpeg_pFormatCtx->streams[i];
			ffmpeg_frameWidth = enc->width;
			ffmpeg_frameHeight = enc->height;

			break;
		}
	}

	if(ffmpeg_videoStreamIndex != -1)
	{
		AVPacket pkt;

		SDL_Surface *scr;
		SDL_Window *window;

		SDL_Init(SDL_INIT_VIDEO);

		window = SDL_CreateWindow("h264motion",
		                          SDL_WINDOWPOS_UNDEFINED,
		                          SDL_WINDOWPOS_UNDEFINED,
		                          (ffmpeg_frameWidth/8) + 2, (ffmpeg_frameHeight/8) + 2,
		                          SDL_WINDOW_SHOWN);
		scr = SDL_GetWindowSurface(window);


		while(1)
		{

			if (av_read_frame(ffmpeg_pFormatCtx, &pkt) == 0)
			{
				if(pkt.stream_index == ffmpeg_videoStreamIndex)
				{

					int ret = 0;
					int got_frame = 0;
					av_frame_unref(ffmpeg_pFrame);

					if ((ret = avcodec_decode_video2(ffmpeg_pVideoStream->codec, ffmpeg_pFrame, &got_frame, &pkt)) >=0)
					{
						ret = FFMIN(ret, pkt.size); /* guard against bogus return values */
						pkt.data += ret;
						pkt.size -= ret;

						if(got_frame > 0)
						{
							int64_t pts;
							char pictType;
							AVFrameSideData* sd = NULL;

							pictType = av_get_picture_type_char(ffmpeg_pFrame->pict_type);
							// fragile, consult fresh f_select.c and ffprobe.c when updating ffmpeg
							pts = ffmpeg_pFrame->pkt_pts != AV_NOPTS_VALUE ? ffmpeg_pFrame->pkt_pts : (ffmpeg_pFrame->pkt_dts != AV_NOPTS_VALUE ? ffmpeg_pFrame->pkt_dts : pts + 1);

							if((sd = av_frame_get_side_data(ffmpeg_pFrame, AV_FRAME_DATA_MOTION_VECTORS)) != NULL)
							{
								AVMotionVector* mvs = (AVMotionVector*)sd->data;
								int mvcount = sd->size / sizeof(AVMotionVector);

								void *pixels;
								int pitch;

								SDL_LockSurface(scr);

								for(int i = 0; i < mvcount; i++)
								{
									uint32_t pixel_data;

									int mvdx = abs(mvs[i].dst_x - mvs[i].src_x) * 20;
									int mvdy = abs(mvs[i].dst_y - mvs[i].src_y) * 20;
									int vector = (((mvdx+mvdy)/2));

									int x = mvs[i].dst_x/16;
									int y = mvs[i].dst_y/16;

									pixel_data = (0xff000000);

									pixel_data = (0x00ffffff) & (((vector&0xff)<<16) | ((mvdy&0xff)<<8) | (mvdx&0xff));

									putpixel2(scr, x, y, pixel_data);
								}


								SDL_UnlockSurface(scr);

								SDL_UpdateWindowSurface(window);

								SDL_PollEvent(NULL);

								usleep(33333);

							}
						}
						else
						{
							av_packet_unref(&pkt);
						}
					}
				}
				else
				{
					av_packet_unref(&pkt);
				}
			}
			else
			{
				break;
			}
		}

		SDL_Quit();

	}
}

