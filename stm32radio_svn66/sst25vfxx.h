#ifndef SST25VFXX_H_INCLUDED
#define SST25VFXX_H_INCLUDED

#include <stdint.h>

/* function list */
extern void sst25vfxx_init(void);
extern uint32_t sst25vfxx_read(uint32_t offset,uint8_t * buffer,uint32_t size);
extern uint32_t sst25vfxx_page_write(uint32_t page,const uint8_t * buffer,uint32_t size);

/* command list */
#define CMD_RDSR                    0x05  /* ��״̬�Ĵ���     */
#define CMD_WRSR                    0x01  /* д״̬�Ĵ���     */
#define CMD_EWSR                    0x50  /* ʹ��д״̬�Ĵ��� */
#define CMD_WRDI                    0x04  /* �ر�дʹ��       */
#define CMD_WREN                    0x06  /* ��дʹ��       */
#define CMD_READ                    0x03  /* ������           */
#define CMD_FAST_READ               0x0B  /* ���ٶ�           */
#define CMD_BP                      0x02  /* �ֽڱ��         */
#define CMD_AAIP                    0xAD  /* �Զ���ַ������� */
#define CMD_ERASE_4K                0x20  /* ��������:4K      */
#define CMD_ERASE_32K               0x52  /* ��������:32K     */
#define CMD_ERASE_64K               0xD8  /* ��������:64K     */
#define CMD_ERASE_full              0xC7  /* ȫƬ����         */
#define CMD_JEDEC_ID                0x9F  /* �� JEDEC_ID      */
#define CMD_EBSY                    0x70  /* ��SOæ���ָʾ */
#define CMD_DBSY                    0x80  /* �ر�SOæ���ָʾ */

/* device id define */
enum _sst25vfxx_id
{
    unknow     = 0,
    SST25VF016 = 0x004125BF,
};


#endif // SST25VFXX_H_INCLUDED
