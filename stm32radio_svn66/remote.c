/*
+----------------------------------------------------
|
| ������ѧϰң��
|
| Chang Logs:
| Date           Author       Notes
| 2010-01-02     aozima       The bate version.
| 2010-02-10     aozima       change printf string ���� to english.
| 2010-03-25     aozima       add remote_fn define.
| 2010-06-16     aozima       add remote_study to ui.
+----------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include <stm32f10x.h>

#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>

/* �ض���printf */
#define printf                   rt_kprintf
/* ��������ƫ��,��λ0.01ms */
#define remote_deviation         15
#define remote_code_len_max      100
/* ���������ļ���ȫ·�� */
#define remote_fn                "/resource/remote.txt"

/* ����ģʽ 0:û����,1:��ѧϰ,2:�������� */
typedef enum
{
    remote_mode_disable,
    remote_mode_study,
    remote_mode_enable,
}remote_mode_type;
remote_mode_type rem_mode = remote_mode_disable;

static unsigned int first_tick = 0;    /* ���ο�ʼ�����ʱ��� */
static unsigned int rx_count   = 0;    /* ���β����в��񵽵��źż���. */
static unsigned short * rm_code = RT_NULL;

/* �ź�������*/
static struct rt_semaphore sem_IR;

struct rem_codes_typedef
{
    unsigned int len;
    unsigned short rem_code[remote_code_len_max];
};
struct rem_codes_typedef * p_rem_code_src = RT_NULL;

static const char  str1[]="KEY_UP";     /* �� */
static const char  str2[]="KEY_DOWN";   /* �� */
static const char  str3[]="KEY_LEFT";   /* �� */
static const char  str4[]="KEY_RIGHT";  /* �� */
static const char  str5[]="KEY_ENTER";  /* ȷ�� */
static const char  str6[]="KEY_RETURN"; /* ���� */
static const char * desc_key[6]= {str1,str2,str3,str4,str5,str6};

/* ������ת���� #####\r\n ��ʽ���ı� */
static void dectoascii(unsigned int date_input,char * p)
{
    p[0] = date_input / 10000 +'0';
    date_input = date_input % 10000;
    p[1] = date_input / 1000  +'0';
    date_input = date_input % 1000;
    p[2] = date_input / 100   +'0';
    date_input = date_input % 100;
    p[3] = date_input / 10    +'0';
    date_input = date_input % 10;
    p[4] = date_input        +'0';
    date_input = 0;
    p[5] = '\r';
    p[6] = '\n';
    p += 7;
}
/* ��#####\r\n ��ʽ�ı�ת�������� */
static unsigned short asciitodec(const char * p_str)
{
    return  ( (p_str[0]-'0')*10000
              + (p_str[1]-'0')*1000
              + (p_str[2]-'0')*100
              + (p_str[3]-'0')*10
              + (p_str[4]-'0') );
}

/* tim5 configure */
static void TIM5_Configuration(void)
{
    /* ʱ�Ӽ���Ƶ���� */
    {
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
        /* Time Base configuration */
        /* 72M/720 = 0.01ms */
        TIM_TimeBaseStructure.TIM_Prescaler = 720-1;
        //����ģʽ:���ϼ���
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        //���¼�������ʼֵ
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

        TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    }

    /* �������� */
    {
        TIM_ICInitTypeDef  TIM_ICInitStructure;

        TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;/* ÿ�μ�⵽��������ʹ���һ�β��� */
        TIM_ICInitStructure.TIM_ICFilter    = 8;/* �˲� */

        TIM_ICInitStructure.TIM_Channel     = TIM_Channel_3;//ѡ��ͨ��3
        TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Falling;//�½���
        TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//ͨ������ѡ��
        TIM_ICInit(TIM5, &TIM_ICInitStructure);

        TIM_ICInitStructure.TIM_Channel     = TIM_Channel_4;//ѡ��ͨ��3
        TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;//������
        TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;//ͨ������ѡ��
        TIM_ICInit(TIM5, &TIM_ICInitStructure);
    }

    /* ���봥��Դѡ��:�ⲿ���봥�� */
    TIM_SelectInputTrigger(TIM5, TIM_TS_ETRF);//TIM_TS_ETRF �ⲿ����
    /* ��ģʽ-��λģʽ */
    /* TIM_SlaveMode_Reset 4:ѡ�еĴ�������(TRGI)�����������³�ʼ�������������Ҳ���һ�����¼Ĵ������ź� */
    TIM_SelectSlaveMode(TIM5, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM5, TIM_MasterSlaveMode_Enable);

    /* TIM enable counter */
    TIM_Cmd(TIM5, ENABLE);

    /* Enable the CC3 and CC4 Interrupt Request */
    TIM_ITConfig(TIM5, TIM_IT_CC3, ENABLE);
    TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the TIM5 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

    /* TIM5 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    /* clock enable */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* TIM5 channel 3 pin (PA.02) configuration */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void rem_start(void)
{
    RCC_Configuration();
    GPIO_Configuration();

    /* configure TIM5 for remote and encoder */
    NVIC_Configuration();
    TIM5_Configuration();

    p_rem_code_src = rt_malloc( sizeof(struct rem_codes_typedef)*6 );
    if( p_rem_code_src != RT_NULL)
    {
        rt_memset(p_rem_code_src,0, sizeof(struct rem_codes_typedef)*6 );

        /* ���������Ϣ */
        {
            int fd,size;
            char buf[7];/* �ļ���ȡ��ʱ���� #####\r\n */
            unsigned int i;
            unsigned short tmp;
            unsigned int read_index = 0;
            unsigned int EOF_flag = 1;

            printf("\r\ndecode remote codes");
            fd = open(remote_fn,O_RDONLY,0);
            if( fd >= 0 )
            {
                printf("\r/resource/remote.txt open succeed.\r\n");
                while( EOF_flag )
                {
                    /* ��ȡ���� */
                    size = read(fd,buf,7);
                    if( (size == 7) && (buf[5]=='\r') && buf[6]=='\n' )
                    {
                        /* ת���õ��������ݳ��� */
                        tmp = asciitodec(buf);
                        if( tmp<100 )
                        {
                            unsigned int code_len = tmp;
                            p_rem_code_src[read_index].len = code_len;

                            /* ����������ȷ��� �Ϳ�ʼ���ļ���ȡ�������� */
                            for(i=0; i<code_len; i++)
                            {
                                size = read(fd,buf,7);
                                if( (size == 7) && (buf[5]=='\r') && buf[6]=='\n' )
                                {
                                    /* ת���õ��������� */
                                    tmp = asciitodec(buf);
                                    p_rem_code_src[read_index].rem_code[i] = tmp;
                                }
                            }
                            read_index++;
                        }
                    }
                    else
                    {
                        EOF_flag = 0;
                    }
                }//while( EOF_flag )

                /* �ж��Ƿ���ȷ������������ļ� */
                if ( p_rem_code_src[0].len > 0 && p_rem_code_src[0].len < remote_code_len_max )
                {
                    /* ���ù���ģʽΪ����ʶ��ģʽ */
                    rem_mode = remote_mode_enable;
                    printf("\r\ndecode succeed,The remote enable\r\n");
                }
                else
                {
                    /* ���ù���ģʽΪ�ر�ģʽ */
                    rem_mode = remote_mode_disable;
                    printf("\r\nrem_codes decode fail,The remote disable\r\n");
                }
            }
            else
            {
                printf("\rrem_codes /resource/remote.txt open fail! fd:%d\r\nThe remote disbale.\r\nplease run rem_study()\r\n",fd);
            }
            close(fd);
        }/* ���������Ϣ */

    }
    else
    {
        rem_mode = remote_mode_disable;
        printf("\r\nmalloc rem_codes[] fail!!!\r\nThe remote disable!");
    }
}

void rem_encoder(struct rtgui_event_kbd * p_kbd_event)
{
    /* ����Ƿ������ݱ����� */
    if( (rem_mode==2) && (rx_count > 0) )
    {
        /* �ֶ������һ�������� */
        rm_code[0] = 0;
        rx_count = 0;

        /* ƥ�䲶������� */
        {
            unsigned int tmp;
            unsigned int err_flag = 0;
            unsigned int rem_cmp_n = 6;

            /* ѭ��ƥ������KEY */
            while( rem_cmp_n )
            {
                unsigned int tmp2 = p_rem_code_src[ 6-rem_cmp_n ].len;
                //printf("\r\nrem_cmp_n:%d  tmp2:%d",rem_cmp_n,tmp2);
                if( tmp2 )
                {

                    for(tmp=0; tmp<tmp2; tmp++)
                    {
                        /* �жϲ������Ƿ���ƫ������Χ�� */
                        if( !( (rm_code[tmp] < p_rem_code_src[6-rem_cmp_n].rem_code[tmp]+remote_deviation)
                                && (rm_code[tmp] > p_rem_code_src[6-rem_cmp_n].rem_code[tmp]-remote_deviation)) )
                        {
                            err_flag = 1;
                        }
                    }
                }
                else
                {
                    err_flag = 1;
                    printf("\r\nThe rem codes len is 0.");
                }

                if( err_flag==0 )
                {
                    /* �Ա�ȫ�����ݷ��� */
                    printf("\r\nmatch key: %s",desc_key[6-rem_cmp_n]);
                    switch( rem_cmp_n )
                    {
                    case 6:
                        p_kbd_event->key  = RTGUIK_UP;
                        break;
                    case 5:
                        p_kbd_event->key  = RTGUIK_DOWN;
                        break;
                    case 4:
                        p_kbd_event->key  = RTGUIK_LEFT;
                        break;
                    case 3:
                        p_kbd_event->key  = RTGUIK_RIGHT;
                        break;
                    case 2:
                        p_kbd_event->key  = RTGUIK_RETURN;
                        break;
                    case 1:
                        p_kbd_event->key  = RTGUIK_HOME;
                        break;
                    default:
                        break;
                    }
                    rem_cmp_n = 0;
                }
                else
                {
                    /* �ԱȲ�����,����������,�Խ�����һ�ζԱ� */
                    err_flag = 0;
                    rem_cmp_n --;
                }

            }
        }
    }/* ����ң��ƥ�� */
}

/* remote isr */
void remote_isr(void)
{
    static unsigned int clr_flag = 1;      /* �Ƿ���Ҫ�������,�����ж��Ƿ���ĳ�β�������. */
    unsigned int tick_now  = rt_tick_get();/* ��ȡ��ǰʱ���.*/

    /* ����ң���½��� */
    if(TIM_GetITStatus(TIM5, TIM_IT_CC3) == SET)
    {
        switch( rem_mode )
        {
        case 0:/* δ���� */
            break;
        case 1:/* ��ѧϰ */
            /* ����ܵĽ��ռ���Ϊ0,���ж�����һ�ο�ʼ,��Ҫ����. */
            if( rx_count==0 )
            {
                //��Ҫ��0
                clr_flag = 1;
            }
            if( rx_count < remote_code_len_max )
            {
                rm_code[rx_count++] = TIM_GetCapture3(TIM5);
            }
            break;
        case 2://��������
            if( ( rx_count>(remote_code_len_max-10) ) || tick_now>first_tick+10 )
            {
                rx_count = 0;
                clr_flag = 1;
            }
            if(rx_count < remote_code_len_max )
            {
                rm_code[rx_count++] = TIM_GetCapture3(TIM5);
            }
            break;
        default:
            rem_mode = remote_mode_disable;/* �쳣����,��رպ���ҡ�� */
            break;
        }
        TIM_ClearITPendingBit(TIM5, TIM_IT_CC3);
    }

    /* ����ң�������� */
    if(TIM_GetITStatus(TIM5, TIM_IT_CC4) == SET)
    {
        switch( rem_mode )
        {
        case 0://δ����
            break;
        case 1://��ѧϰ
            if( rx_count < remote_code_len_max )
            {
                rm_code[rx_count++] = TIM_GetCapture4(TIM5);
            }
            break;
        case 2://��������
            if( rx_count < remote_code_len_max )
            {
                rm_code[rx_count++] = TIM_GetCapture4(TIM5);
                if( p_rem_code_src[0].len == rx_count)
                {
                    rt_sem_release(&sem_IR);
                }
            }
            break;
        default:
            rem_mode = remote_mode_disable;/* �쳣����,��رպ���ҡ�� */
            break;
        }
        TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);
    }

    /* ����ʱ��� */
    first_tick = tick_now;
    /* ����Ƿ���Ҫ���ü����� */
    if( clr_flag )
    {
        /* ���ü����� */
        TIM_SetCounter(TIM5,0);
        clr_flag = 0;
    }
}

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/workbench.h>
#include <rtgui/widgets/view.h>

#include "player_bg.h"

static rtgui_view_t*           setting_view  = RT_NULL;
static rtgui_workbench_t* father_workbench   = RT_NULL;

/* ��������ѧϰ���� */
void remote_study_thread_entry(void * parameter)
{
    struct rtgui_event_command ecmd;

    unsigned int i;

    int fd,size;
    char * tmp_buf = RT_NULL;

    tmp_buf = rt_malloc( (remote_code_len_max+1)*7 );
    if(tmp_buf == RT_NULL) return;

    RTGUI_EVENT_COMMAND_INIT(&ecmd);
    ecmd.type = RTGUI_EVENT_PAINT;

    rem_mode = remote_mode_study;
    rx_count = 0;
    printf("\r\nremote studing.....");
    fd = open(remote_fn,O_WRONLY | O_TRUNC,0);
    if( fd >= 0 )
    {
        printf("\r\n/resource/remote.txt create succeed.");
    }
    else
    {
        printf("\r/resource/remote.txt create fail.\r\nabort.");
        return;
    }

    /* ѧϰ6������ */
    for( i=0; i<6; i++)
    {
        unsigned int is_ok = 1;
        memset(rm_code,0,sizeof(rm_code));
        printf("\r\npress key %s",desc_key[i]);

        //��UI������Ϣ
        ecmd.command_id = PLAYER_REQUEST_REMOTE;
        strncpy(ecmd.command_string,desc_key[i],RTGUI_NAME_MAX);
        rtgui_thread_send(rt_thread_find("ply_ui"), &ecmd.parent, sizeof(ecmd));

        while( is_ok==1 )
        {
            /* �����200ms���в�������. */
            if( ( rem_mode== remote_mode_study ) && (rt_tick_get()>first_tick+20) && (rx_count > 0) )
            {
                unsigned int a,rx_count_current;
                char * p = tmp_buf;

                printf("\r\n%s",desc_key[i]);

                printf("  rx_count : %d",rx_count);

                rm_code[0] = 0;

                /* �Ӳ��񵽵�������ȡ�õ�һ����Ч�εĳ���. */
                {
                    unsigned int i = 0;
                    rx_count_current = 0;
                    while( rx_count_current==0 )
                    {
                        /* ����˵�����µ�С��20ms,���µ㲻Ϊ0,����Ϊ��Ч. */
                        if( ((rm_code[i]+2000)>rm_code[i+1] ) && (rm_code[i+1] != 0) )
                        {
                            i++;
                        }
                        else /* ���������Ч.*/
                        {
                            /* ��ñ��β������Ч��¼��. */
                            rx_count_current = i+1;
                        }
                    }
                    printf("  rx_count_current : %d",rx_count_current);
                }
                p_rem_code_src[i].len = rx_count_current;

                /* TIM disable counter */
                TIM_Cmd(TIM5, DISABLE);
                /* disable the CC3 and CC4 Interrupt Request */
                TIM_ITConfig(TIM5, TIM_IT_CC3, DISABLE);
                TIM_ITConfig(TIM5, TIM_IT_CC4, DISABLE);

                /* �ѱ��β������Ч��¼��ת����ʮ����ASCII����. */
                dectoascii(rx_count_current,p);
                p += 7;

                for( a=0; a<rx_count_current; a++)
                {
                    /* �ѵ�ǰ����ֱ��д����Ʒ���� */
                    p_rem_code_src[i].rem_code[a] = rm_code[a];
                    /* Ȼ��ת�����ı���ʽ #####\r\n */
                    dectoascii(rm_code[a],p);
                    p += 7;
                }
                size = write(fd,(char*)tmp_buf,(rx_count_current+1)*7 );
                if( size==((rx_count_current+1)*7) )
                {
                    printf(" file write succeed!");
                    is_ok++;
                    rt_thread_delay( 2 );

                    rx_count = 0;//������ռ���,�Ա�����ٴβ���

                    /* ���´� TIM5 ���в��� */
                    TIM_ClearITPendingBit(TIM5, TIM_IT_CC3);
                    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);
                    /* TIM ENABLE counter */
                    TIM_Cmd(TIM5, ENABLE);
                    /* ENABLE the CC3 and CC4 Interrupt Request */
                    TIM_ITConfig(TIM5, TIM_IT_CC3, ENABLE);
                    TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);
                }
                else
                {
                    printf(" file write fail.\r\nabort.");
                    return;
                }
            }
            rt_thread_delay(1);
        }//while( is_ok==1 )
    }//for( i=0; i<6; i++)
    close(fd);
    printf("\r\nremote study complete.The remote enable.\r\n");

    strcpy(ecmd.command_string,"done");
    rtgui_thread_send(rt_thread_find("ply_ui"), &ecmd.parent, sizeof(ecmd));
    rt_thread_delay(RT_TICK_PER_SECOND);
    strcpy(ecmd.command_string,"exit");
    rtgui_thread_send(rt_thread_find("ply_ui"), &ecmd.parent, sizeof(ecmd));

    rem_mode = remote_mode_enable;

    rt_free(tmp_buf);
    return;
}


static unsigned int yy2 = 0;
static rt_bool_t view_event_handler ( struct rtgui_widget* widget, struct rtgui_event* event )
{
    switch ( event->type )
    {
    case RTGUI_EVENT_PAINT:
    {
        struct rtgui_dc* dc;
        struct rtgui_rect rect;
        char* line;

        line = rtgui_malloc(256);

        //��ʼ��ͼ
        dc = rtgui_dc_begin_drawing ( widget );

        if ( dc == RT_NULL )
            return RT_FALSE;

        //�õ�λ��
        rtgui_widget_get_rect ( widget, &rect );

        /* fill background */
        rtgui_dc_fill_rect(dc, &rect);

        rect.y2 = rect.y1 + 18;
        yy2 = rect.y2;

        sprintf(line, "����ң��ѧϰ����");
        rtgui_dc_draw_text(dc, line, &rect);

        rect.y1 = rect.y2;
        rect.y2 = rect.y1 + 18;
        yy2 = rect.y2;

        sprintf(line, "����ENTER������");
        rtgui_dc_draw_text(dc, line, &rect);

        rtgui_dc_end_drawing ( dc );
        rtgui_free(line);

        return RT_FALSE;
    }
    case RTGUI_EVENT_COMMAND:
    {
        struct rtgui_dc* dc;
        struct rtgui_rect rect;
        char* line;

        struct rtgui_event_command* ecmd = (struct rtgui_event_command*)event;
//        rt_kprintf("cmd type:%d cmd id:%d cmd_str: %s",ecmd->type,ecmd->command_id,ecmd->command_string);
        if( (strcmp(ecmd->command_string,"done")==0) || (strcmp(ecmd->command_string,"exit")==0))
        {
            if( strcmp(ecmd->command_string,"done")==0 )
            {
                line = rtgui_malloc(256);
                dc = rtgui_dc_begin_drawing ( widget );
                rtgui_widget_get_rect ( widget, &rect );
                rect.y1 = yy2;
                rect.y2 = rect.y1 + 18;
                yy2 = rect.y2;
                sprintf(line, "����ѧϰ���,�Ѵ�ң�ع���");
                rtgui_dc_draw_text(dc, line, &rect);
                rtgui_dc_end_drawing ( dc );
                rtgui_free(line);
            }
            else
            {
                rtgui_workbench_t* workbench;

                workbench = RTGUI_WORKBENCH ( RTGUI_WIDGET ( setting_view )->parent );
                rtgui_workbench_remove_view ( workbench, setting_view );

                rtgui_view_destroy ( setting_view );
                setting_view = RT_NULL;
                return RT_TRUE;
            }
        }
        else
        {
            line = rtgui_malloc(256);
            dc = rtgui_dc_begin_drawing ( widget );
            rtgui_widget_get_rect ( widget, &rect );
            rect.y1 = yy2;
            rect.y2 = rect.y1 + 18;
            yy2 = rect.y2;

            sprintf(line, "�밴��:%s",ecmd->command_string);
            rtgui_dc_draw_text(dc, line, &rect);
            rtgui_dc_end_drawing ( dc );
            rtgui_free(line);

            return RT_TRUE;
        }

    }
    case RTGUI_EVENT_KBD:
    {
        struct rtgui_event_kbd* ekbd;

        ekbd = ( struct rtgui_event_kbd* ) event;

        if ( ekbd->type == RTGUI_KEYDOWN && ekbd->key == RTGUIK_RETURN )
        {
            rtgui_workbench_t* workbench;

            workbench = RTGUI_WORKBENCH ( RTGUI_WIDGET ( setting_view )->parent );
            rtgui_workbench_remove_view ( workbench, setting_view );

            rtgui_view_destroy ( setting_view );
            setting_view = RT_NULL;
            return RT_TRUE;
        }
        return RT_FALSE;
    }
    }
    return rtgui_view_event_handler ( widget, event );
}

void remote_study_ui(rtgui_workbench_t* workbench)
{
    father_workbench = workbench;

    setting_view = rtgui_view_create ( "setting_view" );
    /* ָ����ͼ�ı���ɫ */
    RTGUI_WIDGET_BACKGROUND ( RTGUI_WIDGET ( setting_view ) ) = green;
    /* this view can be focused */
    RTGUI_WIDGET ( setting_view )->flag |= RTGUI_WIDGET_FLAG_FOCUSABLE;

    //���÷�����
    rtgui_widget_set_event_handler ( RTGUI_WIDGET ( setting_view ), view_event_handler );

    /* ��ӵ���workbench�� */
    rtgui_workbench_add_view ( father_workbench, setting_view );
    /* ��ģʽ��ʽ��ʾ��ͼ */
    rtgui_view_show ( setting_view, RT_FALSE );

    //����ѧϰ�߳�
    {
        rt_thread_t remote_study_thread;
        remote_study_thread = rt_thread_create("rm_study",
                                               remote_study_thread_entry, RT_NULL,
                                               2048, 30, 2);
        if (remote_study_thread != RT_NULL)rt_thread_startup(remote_study_thread);
    }
}

static struct rtgui_event_kbd kbd_event;
static void remote_thread_entry(void *parameter)
{
    /* init keyboard event */
    RTGUI_EVENT_KBD_INIT(&kbd_event);
    kbd_event.mod  = RTGUI_KMOD_NONE;
    kbd_event.unicode = 0;

    while(1)
    {
        /* �ȴ��ź���,�ź����ڲ���һ�������ݺ��ͷ� */
        if (rt_sem_take(&sem_IR,RT_WAITING_FOREVER) == RT_EOK)
        {
            kbd_event.key = RTGUIK_UNKNOWN;
            if( rem_mode == 2)
            {
                rem_encoder(&kbd_event);
            }
            if( kbd_event.key != RTGUIK_UNKNOWN)
            {
                kbd_event.type = RTGUI_KEYDOWN;
                /* post down event */
                rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));
                rt_thread_delay(20);

                /* post up event */
                kbd_event.type = RTGUI_KEYUP;
                rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));
            }
        }
    }
}

void remote_init(void)
{
    rt_thread_t remote_thread;

    rm_code = rt_malloc( remote_code_len_max*2 );
    if (rm_code == RT_NULL) return;

    rt_sem_init(&sem_IR,"sem_IR", 0,RT_IPC_FLAG_FIFO);

    rem_start();

    remote_thread = rt_thread_create("remote",
                                     remote_thread_entry, RT_NULL,
                                     384, 30, 2);
    if (remote_thread != RT_NULL)rt_thread_startup(remote_thread);
}
