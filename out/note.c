/*
1���ٽ������ã���ֹϵͳ���ȣ�������ֹ�ж�
2���Ƚ���ҵ���߼�����ͼ����д����
3����������д��
	1�����ٽ���
	2��uart_write
	3���ȴ������ж���ɺ��жϴ����������ͷŻ����ź���
	4���˳��ٽ���
4��ת�Ʊ�дfsm
5���̵�����������
	1������PI��PID��������ʹϵͳ������̬���������ھ���
	2���رտ�������ʹ�ÿ����ź�ֱ�Ӽ���ִ�л�����ʹϵͳ��
	3�����ݾ��鹫ʽ����Kp��Ki��Kd
	4�����������´�������������ݷ������ߣ�΢������
	5��ʹ��������ʽ�����������趨ֵӰ�죬��ʹ�õ�ͨ�˲�������ƽ���˲���������
6����ũ�������ɣ�����Ƶ��>=2*fmax���ź�Ƶ�ʣ�
7���ֲ����˼�룺���·��롢���ҷ���
8���ֲ����ԭ��
	1��������ʹ��ȫ�ֱ���
	2���ӿ������²���ù�ϵ�������ٿ�����
	3��ͬ���ģ����⻥����ã���Ҫ���ݶ��ǴӸ߲�ӿڻ�ȡ
9��Ƕ��ʽ�����ܹ�
	Ӧ�ò㣺���ò�ͬ��ҵ��ģ������߼�����
	ҵ���߼��㣺����ҵ��ͨ��ҵ�񡢱�������ҵ���
	����ģ��㣺1��API���ṩ���ϲ����
				2������ģ���
	Ӳ�������㣺1��API���ṩ���ϲ����
				2��������������
10�����ADC�����ȶ���
	1�������������ô�һЩ
	2���ϵ���ӳ�һ��ʱ����У׼ADC
	3�����ʵ��˲��㷨
11��PID������η�
	1��ʹϵͳ���ڴ���������ƣ����趨ֵ����Ϊϵͳ���ֵ��60%~70%����Kp��0������ֱ�������𵴣������о����
		Ȼ�󷴹������𽥼�СKp��ֱ������ʧ�������о������ʱ���趨��KpֵΪ��ǰKpֵ��60%~70%��
	2��ϵͳ����������Ti�����ֵ�𽥼�С��ֱ�������𵴣�Ȼ�󷴹�����������Ti��ֱ������ʧ����ʱ���趨��TiֵΪ
		��ǰTiֵ��150%~180%��
	3��ϵͳ����΢���Td�ĵ��η�ʽ��Kp��Tiһ�����趨ֵΪ����ʱ����ǰTdֵ��30%��
	4����󣬿�����΢����
	5��˫��PID���Σ��������ڻ�(��ʹ�⻷�رգ��������ڻ�����)���������⻷��
12��PWM����ʱ�䣺PWM���ʱ��Ϊ��ʹH�Ż��H�ŵ�·�����¹ܲ�����ΪMOS�ܿ����ٶ����ⷢ�����¹�ͬʱ��ͨ�����õı���ʱ�䡣
	����ʱ���ڣ�PWMͨ���������PWM��Ч���ԣ����磺�������Ļ���PWM
13������H��ʽ��·����Ҫʹ�û���PWM���������·���������Ҫʹ��CH1��CH1N��CH2��CH2N��4��IO��
14��Ƶ����д����ʹ��DMA���Ĳ���������ʹ��ccmram�����Ż����磺
	������rt_uint8_t buff[1024] __attribute__((section("ccmram"))) = {0};
	������__attribute__((section("ccmram"))) void func(void)
15������ָ��ת����ʽ��_P_CONVERT(_ADDR, _TYPE)				(*(TYPE *)(uint32_t)(_ADDR))	
16��volatile�÷���
	1�����ڴ��н��е�ַӳ����豸�Ĵ���
	2�����жϴ��������п��ܱ��޸ĵ�ȫ�ֱ���
	3�����߳�Ӧ���е�ȫ�ֱ���
17���͹���Ӧ�ã�
	1�����͹���ģʽ
	2���������ó����ģʽ
	3������ʱ�ӹر�
18���������Ըй�gamaϵ��Ϊ1/2.2����LED��gamaУ��ϵ��Ϊ2.2����ʽ��POWER(X,2.2)*255(�Լ����ֽ�������ΧΪ0~1��step=1/����)
19�����ʽ
	1���ֲ����
	2������ָ�롢�������ص���ת�Ʊ���״̬��
	3������
20��һ�׿������˲�
	1��Last_P��Ӱ����������ٶȣ�һ���趨��Сֵ
	2��Q��ԽСԽ����ģ��Ԥ��ֵ������̫С���ײ�����ɢ
	3��R��ԽСԽ����������ҪӰ�������ٶȣ�����̫С���ײ�����ɢ
/*