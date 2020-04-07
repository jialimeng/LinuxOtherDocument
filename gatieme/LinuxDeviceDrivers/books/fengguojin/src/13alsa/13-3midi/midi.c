char *device_in;
char *device_out;
int fd_in = -1,fd_out = -1;
snd_rawmidi_t *handle_in = 0,*handle_out = 0;
if (device_in) {
	err = snd_rawmidi_open(&handle_in,NULL,device_in,0);	//��MIDI�豸
	if (err) {
		fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_in,err);
	}
}
if (device_out) {
	err = snd_rawmidi_open(NULL,&handle_out,device_out,0);
	if (err) {
		fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_out,err);
	}
}
if ((handle_in) && (handle_out))
{
	while (!stop) 
	{
		unsigned char ch;
		snd_rawmidi_read(handle_in,&ch,1);//��MIDI�豸
		snd_rawmidi_write(handle_out,&ch,1);//дMIDI�豸
		snd_rawmidi_drain(handle_out);
	}
}
if (handle_in)
{
	snd_rawmidi_drain(handle_in);
	snd_rawmidi_close(handle_in); //�ر�midi�����豸
}
if (handle_out) 
{
	snd_rawmidi_drain(handle_out); 
	snd_rawmidi_close(handle_out);//�ر�midi����豸
}
