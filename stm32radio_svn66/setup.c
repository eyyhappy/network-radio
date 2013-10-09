#include <rtthread.h>
#include <dfs_posix.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "codec.h"
#include "setup.h"     // RADIO��������
#include "drawpad.h"   // �򵥵Ļ�ͼ��
#include "utils.h"

#define setup_fn    "/setup.ini"

setup_TypeDef radio_setup;

static const char  kn_volume[]      = "default_volume";
static const char  kn_brightness[]  = "lcd_brightness";
static const char  kn_touch_min_x[] = "touch_min_x";
static const char  kn_touch_max_x[] = "touch_max_x";
static const char  kn_touch_min_y[] = "touch_min_y";
static const char  kn_touch_max_y[] = "touch_max_y";

static void load_default(void)
{
    rt_kprintf("load_default!\r\n");
    radio_setup.default_volume = 25;
    radio_setup.lcd_brightness = 50;

    radio_setup.touch_min_x = 194;
    radio_setup.touch_max_x = 1810;
    radio_setup.touch_min_y = 1876;
    radio_setup.touch_max_y = 222;

    save_setup();
}

rt_err_t load_setup(void)
{
    int fd, length;
    char line[64];

    fd = open(setup_fn, O_RDONLY, 0);
    if (fd >= 0)
    {
        length = read_line(fd, line, sizeof(line));
        if (strcmp(line, "[config]") == 0)
        {
            char* begin;

            // default_volume
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_volume, sizeof(kn_volume) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.default_volume = atoi(begin);
            }

            // lcd_brightness
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_brightness, sizeof(kn_brightness) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.lcd_brightness = atoi(begin);
            }

            // touch_min_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_x, sizeof(kn_touch_min_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_min_x = atoi(begin);
            }

            // touch_max_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_x, sizeof(kn_touch_max_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_max_x = atoi(begin);
            }

            // touch_min_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_y, sizeof(kn_touch_min_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_min_y = atoi(begin);
            }

            // touch_max_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_y, sizeof(kn_touch_max_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_max_y = atoi(begin);
            }

        }
        else
        {
            close(fd);
            load_default();
            return RT_EOK;
        }
    }
    else
    {
        load_default();
    }

    close(fd);
    return RT_EOK;
}

rt_err_t save_setup(void)
{
    int fd, size;
    char* p_str;
    char* buf = rt_malloc(1024);

    if (buf == RT_NULL)
    {
        rt_kprintf("no memory\r\n");
        return RT_ENOMEM;
    }

    p_str = buf;

    //������Ч�Լ��,��ֹȫ�ڻ�����.
    if (radio_setup.default_volume < 5)radio_setup.default_volume = 5;
    if (radio_setup.lcd_brightness < 5)radio_setup.lcd_brightness = 5;

    fd = open(setup_fn, O_WRONLY | O_TRUNC, 0);
    if (fd >= 0)
    {
        size = sprintf(p_str, "[config]\r\n"); // [config] sprintf(p_str,"")
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_volume, radio_setup.default_volume); //default_volume
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_brightness, radio_setup.lcd_brightness); //lcd_brightness
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_x, radio_setup.touch_min_x); //touch_min_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_x, radio_setup.touch_max_x); //touch_max_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_y, radio_setup.touch_min_y); //touch_min_y
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_y, radio_setup.touch_max_y); //touch_max_y
        p_str += size;
    }

    size = write(fd, buf, p_str - buf);
    if (size == (p_str - buf))
    {
        rt_kprintf("file write succeed:\r\n");
    }

    close(fd);
    rt_free(buf);

    return RT_EOK;
}

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/image.h>
#include <rtgui/image_hdc.h>
#include <rtgui/widgets/workbench.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/list_view.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/slider.h>
#include <rtgui/widgets/button.h>

static struct rtgui_list_view* function_view = RT_NULL;
static rtgui_workbench_t* father_workbench   = RT_NULL;

//���ٱ�view�����ϼ��˵�
static void function_action_return(struct rtgui_widget* widget, void* paramter)
{
    rtgui_workbench_remove_view(father_workbench, RTGUI_VIEW(function_view));
    rtgui_view_destroy(RTGUI_VIEW(function_view));
    function_view = RT_NULL;
}

#if ( LCD_VERSION == 2 ) || ( LCD_VERSION == 3 )
extern void calibration_init(void);
static void function_calibration(struct rtgui_widget* widget, void* parameter)
{
    calibration_init();
}
#endif

static void function_remote_study(struct rtgui_widget* widget, void* paramter)
{
    extern void remote_study_ui(rtgui_workbench_t * workbench);
    remote_study_ui(father_workbench);
}

static void function_drawpad(struct rtgui_widget* widget, void* paramter)
{
    function_sketchpad(father_workbench);
}

//------------- �����ͱ����������� --------------------
static rtgui_slider_t* slider_volume = RT_NULL, *slider_brightness = RT_NULL;

static rt_bool_t view_event_handler(struct rtgui_widget* widget, struct rtgui_event* event)
{
    if (event->type == RTGUI_EVENT_KBD)
    {
        struct rtgui_event_kbd* ekbd;
        ekbd = (struct rtgui_event_kbd*) event;

        if (ekbd->type == RTGUI_KEYDOWN && ekbd->key == RTGUIK_RETURN)
        {
            rtgui_view_t* view;
            rtgui_workbench_t* workbench;

            /* remove view in workbench */
            view = RTGUI_VIEW(widget);
            workbench = RTGUI_WORKBENCH(RTGUI_WIDGET(view)->parent);
            rtgui_workbench_remove_view(workbench, view);
            rtgui_view_destroy(view);
            return RT_TRUE;
        }
        return RT_FALSE;
    }
    return rtgui_view_event_handler(widget, event);
}

static void cancel_handler(struct rtgui_widget* widget, rtgui_event_t* event)
{
    rtgui_view_t* view;
    rtgui_workbench_t* workbench;

    /* remove view in workbench */
    view = RTGUI_VIEW(widget->parent);
    workbench = RTGUI_WORKBENCH(RTGUI_WIDGET(view)->parent);
    rtgui_workbench_remove_view(workbench, view);
    rtgui_view_destroy(view);
}

static void apply_handler(struct rtgui_widget* widget, rtgui_event_t* event)
{
    extern void brightness_set(unsigned int value);

    rt_uint32_t vol, bri;
    vol = rtgui_slider_get_value(slider_volume);
    bri = rtgui_slider_get_value(slider_brightness);

    //���±���
    brightness_set(bri);

    //��������
    {
        rt_device_t dev = RT_NULL;
        dev = rt_device_find("snd");
        dev->control(dev, CODEC_CMD_VOLUME, &vol);
    }
}

static void save_handler(struct rtgui_widget* widget, rtgui_event_t* event)
{
    extern void brightness_set(unsigned int value);

    rt_uint32_t vol, bri;
    vol = rtgui_slider_get_value(slider_volume);
    bri = rtgui_slider_get_value(slider_brightness);

    //���±���
    brightness_set(bri);

    //��������
    {
        rt_device_t dev = RT_NULL;
        dev = rt_device_find("snd");
        dev->control(dev, CODEC_CMD_VOLUME, &vol);
    }

    //��������
    radio_setup.default_volume = vol;
    radio_setup.lcd_brightness = bri;
    save_setup();

    //�������,���ٱ�����
    {
        rtgui_view_t* view;
        rtgui_workbench_t* workbench;

        /* remove view in workbench */
        view = RTGUI_VIEW(widget->parent);
        workbench = RTGUI_WORKBENCH(RTGUI_WIDGET(view)->parent);
        rtgui_workbench_remove_view(workbench, view);
        rtgui_view_destroy(view);
    }
}

static void function_action_setup(struct rtgui_widget* widget, void* paramter)
{
    rtgui_view_t* view;
    rtgui_rect_t rect;
    rtgui_label_t* label;
    struct rtgui_button* cancel_button, *apply_button, *save_button;
    rtgui_workbench_t* workbench = father_workbench;

    /* create a demo view */
    view = rtgui_view_create("Slider View");
    /* ��ӵ���workbench�� */
    rtgui_workbench_add_view(workbench, view);

    //���÷�����
    rtgui_widget_set_event_handler(RTGUI_WIDGET(view), view_event_handler);

    /* get demo view rect */
    rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);
    rtgui_widget_rect_to_device(RTGUI_WIDGET(view), &rect);
    label = rtgui_label_create("Ĭ������:");
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
    rect.x1 += 5; rect.x2 -= 5;
    rect.y1 += 5; rect.y2 = rect.y1 + 18;
    rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);

    rect.y1 += 20; rect.y2 = rect.y1 + 18;
    slider_volume = rtgui_slider_create(0, 100, RTGUI_HORIZONTAL);
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(slider_volume));
    rtgui_widget_set_rect(RTGUI_WIDGET(slider_volume), &rect);
    rtgui_slider_set_value(slider_volume, radio_setup.default_volume);


    /* get demo view rect */
    rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);
    rtgui_widget_rect_to_device(RTGUI_WIDGET(view), &rect);
    label = rtgui_label_create("LCD��������:");
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
    rect.x1 += 5; rect.x2 -= 5;
    rect.y1 += 50; rect.y2 = rect.y1 + 18;
    rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);

    rect.y1 += 20; rect.y2 = rect.y1 + 18;
    slider_brightness = rtgui_slider_create(0, 100, RTGUI_HORIZONTAL);
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(slider_brightness));
    rtgui_widget_set_rect(RTGUI_WIDGET(slider_brightness), &rect);
    rtgui_slider_set_value(slider_brightness, radio_setup.lcd_brightness);

    //�õ� "ȡ��" ������λ��
    rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);
    rtgui_widget_rect_to_device(RTGUI_WIDGET(view), &rect);
    rect.x1 += 20;
    rect.x2 = rect.x1 + 60;
    rect.y2 -= 20;
    rect.y1 = rect.y2 - 20;

    //���� "ȡ��" ��ť(�淵��)
    cancel_button = rtgui_button_create("ȡ��");
    /* ����onbutton������demo_view_next���� */
    rtgui_button_set_onbutton(cancel_button, cancel_handler);
    /* ���ð�ť��λ����Ϣ */
    rtgui_widget_set_rect(RTGUI_WIDGET(cancel_button), &rect);
    /* ��Ӱ�ť����ͼ�� */
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(cancel_button));


    rect.x1 = rect.x2 + 10;
    rect.x2 = rect.x1 + 60;
    apply_button = rtgui_button_create("Ӧ��");
    /* ����onbutton������demo_view_next���� */
    rtgui_button_set_onbutton(apply_button, apply_handler);
    /* ���ð�ť��λ����Ϣ */
    rtgui_widget_set_rect(RTGUI_WIDGET(apply_button), &rect);
    /* ��Ӱ�ť����ͼ�� */
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(apply_button));


    //�õ� "����" ������λ��
    rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);
    rtgui_widget_rect_to_device(RTGUI_WIDGET(view), &rect);
    rect.x2 -= 20;
    rect.x1 = rect.x2 - 60;
    rect.y2 -= 20;
    rect.y1 = rect.y2 - 20;

    /* ���� "����" ��ť */
    save_button = rtgui_button_create("����");
    /* ����onbutton������demo_view_prev���� */
    rtgui_button_set_onbutton(save_button, save_handler);
    /* ���ð�ť��λ����Ϣ */
    rtgui_widget_set_rect(RTGUI_WIDGET(save_button), &rect);
    /* ��Ӱ�ť����ͼ�� */
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(save_button));

    rtgui_view_show(view, RT_FALSE);
}

static const struct rtgui_list_item function_list[] =
{
    {"�����ϼ��˵�", RT_NULL, function_action_return, RT_NULL},
    {"������������", RT_NULL, function_action_setup, RT_NULL},
#if ( LCD_VERSION == 2 ) || ( LCD_VERSION == 3 )
    {"������У׼", RT_NULL, function_calibration, RT_NULL},
    {"����������", RT_NULL, function_drawpad, RT_NULL},
#endif
    {"ң����ѧϰ", RT_NULL, function_remote_study, RT_NULL},
};


void setting_ui(rtgui_workbench_t* workbench)
{
    rtgui_rect_t rect;

    father_workbench = workbench;

    /* add function view */
    rtgui_widget_get_rect(RTGUI_WIDGET(workbench), &rect);
    function_view = rtgui_list_view_create(function_list,
                                           sizeof(function_list) / sizeof(struct rtgui_list_item),
                                           &rect,
                                           RTGUI_LIST_VIEW_LIST);
    rtgui_workbench_add_view(workbench, RTGUI_VIEW(function_view));
    rtgui_view_show(RTGUI_VIEW(function_view), RT_FALSE);
}
