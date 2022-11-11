#include "fifo.h"

/**
  * @brief  ע��һ��fifo
  * @param  pfifo: fifo�ṹ��ָ��
		    pfifo_buf: fifo�ڴ��
		    size: ����
  * @retval none
*/
void fifo_register(_fifo_t *pfifo, uint8_t *pfifo_buf, uint32_t size,
                   lock_fun lock, lock_fun unlock)
{
	pfifo->buf_size = size;
	pfifo->buf 	= pfifo_buf;
	pfifo->pwrite = pfifo->buf;
	pfifo->pread  = pfifo->buf;
    pfifo->occupy_size = 0;
    pfifo->lock = lock;
    pfifo->unlock = unlock;
}
 
/**
  * @brief  �ͷ�fifo
  * @param  pfifo: fifo�ṹ��ָ��
  * @retval none
*/
void fifo_release(_fifo_t *pfifo)
{
	pfifo->buf_size = 0;
    pfifo->occupy_size = 0;
	pfifo->buf 	= NULL;
	pfifo->pwrite = 0;
	pfifo->pread  = 0;
    pfifo->lock = NULL;
    pfifo->unlock = NULL; 
}
 
/**
  * @brief  ��fifoд����
  * @param  pfifo: fifo�ṹ��ָ��
		    pbuf: ��д����
		    size: ��д���ݴ�С
  * @retval ʵ��д��С
*/
uint32_t fifo_write(_fifo_t *pfifo, const uint8_t *pbuf, uint32_t size)
{
	uint32_t w_size= 0,free_size = 0;
	
	if ((size==0) || (pfifo==NULL) || (pbuf==NULL))
	{
		return 0;
	}
 
    free_size = fifo_get_free_size(pfifo);
    if(free_size == 0)
    {
        return 0;
    }

    if(free_size < size)
    {
        size = free_size;
    }
	w_size = size;
    if (pfifo->lock != NULL)
        pfifo->lock();
	while(w_size-- > 0)
	{
		*pfifo->pwrite++ = *pbuf++;
		if (pfifo->pwrite >= (pfifo->buf + pfifo->buf_size)) 
		{
			pfifo->pwrite = pfifo->buf;
		}
        pfifo->occupy_size++;
	}
    if (pfifo->unlock != NULL)
        pfifo->unlock();
	return size;
}
 
/**
  * @brief  ��fifo������
  * @param  pfifo: fifo�ṹ��ָ��
		    pbuf: �������ݻ���
		    size: �������ݴ�С
  * @retval ʵ�ʶ���С
*/
uint32_t fifo_read(_fifo_t *pfifo, uint8_t *pbuf, uint32_t size)
{
	uint32_t r_size = 0,occupy_size = 0;
	
	if ((size==0) || (pfifo==NULL) || (pbuf==NULL))
	{
		return 0;
	}
    
    occupy_size = fifo_get_occupy_size(pfifo);
    if(occupy_size == 0)
    {
        return 0;
    }

    if(occupy_size < size)
    {
        size = occupy_size;
    }
    if (pfifo->lock != NULL)
        pfifo->lock();
	r_size = size;
	while(r_size-- > 0)
	{
		*pbuf++ = *pfifo->pread++;
		if (pfifo->pread >= (pfifo->buf + pfifo->buf_size)) 
		{
			pfifo->pread = pfifo->buf;
		}
        pfifo->occupy_size--;
	}
    if (pfifo->unlock != NULL)
        pfifo->unlock();
	return size;
}
 
/**
  * @brief  ��ȡfifo�ռ��С
  * @param  pfifo: fifo�ṹ��ָ��
  * @retval fifo��С
*/
uint32_t fifo_get_total_size(_fifo_t *pfifo)
{
	if (pfifo==NULL)
		return 0;
	
	return pfifo->buf_size;
}
 
/**
  * @brief  ��ȡfifo���пռ��С
  * @param  pfifo: fifo�ṹ��ָ��
  * @retval ���пռ��С
*/
uint32_t fifo_get_free_size(_fifo_t *pfifo)
{
	uint32_t size;
 
	if (pfifo==NULL)
		return 0;
	
    size = pfifo->buf_size - fifo_get_occupy_size(pfifo);

	return size;
}
 
/**
  * @brief  ��ȡfifo���ÿռ��С
  * @param  pfifo: fifo�ṹ��ָ��
  * @retval fifo���ô�С
*/
uint32_t fifo_get_occupy_size(_fifo_t *pfifo)
{
	if (pfifo==NULL)
		return 0;
    
	return  pfifo->occupy_size;
}


