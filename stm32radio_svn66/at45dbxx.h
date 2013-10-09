#ifndef AT45DBXX_H_INCLUDED
#define AT45DBXX_H_INCLUDED

/*
user for AT45DB161.
copy form : http://www.ourdev.cn/bbs/bbs_content.jsp?bbs_sn=737106
thanks to gxlujd.
*/
#define AT45DB_BUFFER_1_WRITE                 0x84	/* д���һ������ */
#define AT45DB_BUFFER_2_WRITE                 0x87	/* д��ڶ������� */
#define AT45DB_BUFFER_1_READ                  0xD4	/* ��ȡ��һ������ */
#define AT45DB_BUFFER_2_READ                  0xD6	/* ��ȡ�ڶ������� */
#define AT45DB_B1_TO_MM_PAGE_PROG_WITH_ERASE  0x83	/* ����һ������������д�����洢��������ģʽ��*/
#define AT45DB_B2_TO_MM_PAGE_PROG_WITH_ERASE  0x86	/* ���ڶ�������������д�����洢��������ģʽ��*/
#define AT45DB_MM_PAGE_TO_B1_XFER             0x53	/* �����洢����ָ��ҳ���ݼ��ص���һ������    */
#define AT45DB_MM_PAGE_TO_B2_XFER             0x55	/* �����洢����ָ��ҳ���ݼ��ص��ڶ�������    */
#define AT45DB_PAGE_ERASE                     0x81	/* ҳɾ����ÿҳ512/528�ֽڣ� */
#define AT45DB_SECTOR_ERASE                   0x7C	/* ����������ÿ����128K�ֽڣ�*/
#define AT45DB_READ_STATE_REGISTER            0xD7	/* ��ȡ״̬�Ĵ��� */
#define AT45DB_MM_PAGE_READ                   0xD2	/* ��ȡ����������ָ��ҳ */
#define AT45DB_MM_PAGE_PROG_THRU_BUFFER1      0x82  /* ͨ��������д���������� */

/* function list */
extern void at45dbxx_init(void);

#endif // AT45DBXX_H_INCLUDED
