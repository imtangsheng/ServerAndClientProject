## ��װ����
1.��װ����,����������
	����֪ͨ
	�ɼ������䲻���㹻�ڴ棬�����������ڴ���������!
	������Ƶ��ʧ��()

2.���ģ���SDKδ��װ,����ʾ�Ҳ���ģ��,���ز��ʧ��


## ����ģʽ˵��
1.֡����:��ѡ������Ӳ��������,����Ϊ���¿���,��ʼ�ɼ�һ֡ͼ������,��ɺ��ȡһ֡ͼ������
2.�д���:���յ��д����źź�,��ʼ�ɼ�һ��ͼ������,���źŵ�����������ͼ��߶�,��������,��Ϊ�ɼ����,��ȡһ֡ͼ������
3.����ģʽ:֡�������д����ر�ʹ��,��ʱ����һ����ʱ����,��ȡһ֡ͼ������
4.֡+�д���:֡�������д���ͬʱʹ��,1)֡�����źź�,��ʼ��ʼ�������ź����� 2)��ʼ�����д����źŽ��вɼ�һ��ͼ������,����������,��Ϊ�ɼ����,��ȡһ֡ͼ������

���ú�,��ǰ���û���Ч,�������δ����,�´�������,�����Ĭ�ϵ�����

�� dvpSavePicture()



dvp2api dvpStatus dvpSavePicture  ( const dvpFrame *  pFrame,  
  const void *  pBuffer,  
  dvpStr  file,  
  dvpInt32  quality  
 )   


��ͼ�񱣴浽�ļ� 
����
[in] pFrame ֡��Ϣ  
[in] pBuffer ͼ�����ݵ��ڴ��׵�ַ��Ŀǰ֧�ֵ����ݸ�ʽ������RAW, RGB24��  
[in] file �ļ���������·����ͼ���ļ��ĸ�ʽ���ļ�����չ����������Ŀǰ֧�ֵ�ͼ���ʽ������bmp, jpeg, jpg, png, tiff, tif, gif, dat(��ͼ������)��  
[in] quality ͼ��Ʒ�ʣ�����jpeg��ʽ��Ч������ȡֵ��Χ��[1,100]  
�μ�dvpGetFrame dvpStreamCallback 

�� dvpSetTargetFormat()



dvp2api dvpStatus dvpSetTargetFormat  ( dvpHandle  handle,  
  dvpStreamFormat  TargetFormat  
 )   


����Ŀ��ͼ���ʽ 
����
[in] handle ������  
[in] TargetFormat Ŀ��ͼ���ʽ  
���ؿ�Ԥ�ϵ�����£���ȷ������DVP_STATUS_OK�μ�dvpSetTargetFormatSel dvpGetTargetFormat ע��ͨ���Ǵ����ֱ�������ԭʼ��ʽ 

dvpSetTriggerSource()



dvp2api dvpStatus dvpSetTriggerSource  ( dvpHandle  handle,  
  dvpTriggerSource  TriggerSource  
 )   


��������Ĵ���Դ 
����
[in] handle ������  
[in] TriggerSource ����Ĵ���Դ  
���ؿ�Ԥ�ϵ�����£���ȷ������DVP_STATUS_OK�μ�dvpGetTriggerSource 

 dvpTriggerFire()



dvp2api dvpStatus dvpTriggerFire  ( dvpHandle  handle )  


������������ź� 
����
[in] handle ������  
���ؿ�Ԥ�ϵ�����£���ȷ������DVP_STATUS_OKע���������ú��������Դ 

/**
*@brief ����Boolean������ֵ
*@param[in] handle �豸���
*@param[in] strKey ���Լ�ֵ
*@param[in] bValue ��Ҫ���õ��豸������ֵ
*/
dvp2api dvpStatus dvpSetBoolValue(IN dvpHandle handle, IN dvpStr strKey, IN bool bValue);


/**
*@brief ��ȡString����ֵ
*@param[in] handle �豸���
*@param[in] strKey ���Լ�ֵ
*@param[out] strValue ��ǰֵ
*@param[in,out] iValueSize �ַ�����С
*/
dvp2api dvpStatus dvpGetStringValue(IN dvpHandle handle, IN dvpStr strKey, OUT char* strValue, IN dvpInt32 iValueSize);

/**
*@brief ��ȡString����ֵ(ͬ����ȡ��ʽ�ӿ�)
*@param[in] handle �豸���
*@param[in] strKey ���Լ�ֵ
*@param[out] strValue ��ǰֵ
*@param[in,out] iValueSize �ַ�����С
*/