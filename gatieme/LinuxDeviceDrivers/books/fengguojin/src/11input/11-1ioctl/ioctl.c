int version;
ioctl(fd, EVIOCGVERSION, &version)��//��ȡ�汾
struct input_devinfo device_info;
ioctl(fd, EVIOCGID, &device_info)��//��ȡ�豸��Ϣ
char name[256]= "Unknown";
ioctl(fd, EVIOCGNAME(sizeof(name)), name)//��ȡ����
uint8_t rel_bitmask[REL_MAX/8 + 1];
ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel_bitmask))//��ȡ֧�ֵ��������
