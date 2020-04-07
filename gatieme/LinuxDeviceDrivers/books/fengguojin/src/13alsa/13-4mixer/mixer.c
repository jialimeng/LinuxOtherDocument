int main(void)
{
	int unmute, chn;
	int al, ar;
	snd_mixer_t *mixer;
	snd_mixer_elem_t * elem;
	snd_mixer_open(&mixer, 0);
	snd_mixer_attach(mixer, "default");
	snd_mixer_selem_register(mixer, NULL, NULL);//ע�������
	snd_mixer_load(mixer);
	for(elem=snd_mixer_first_elem(mixer); 
	elem; elem=snd_mixer_elem_next(elem))
    {
		/* �趨�����ķ�Χ 0 ~ 100 */
		snd_mixer_selem_set_playback_volume_range
			(elem, 0, 100);
		/* ȡ���������������� */
		snd_mixer_selem_get_playback_volume
			(elem, SND_MIXER_SCHN_FRONT_LEFT, &al);
		snd_mixer_selem_get_playback_volume
			(elem, SND_MIXER_SCHN_FRONT_RIGHT, &ar);
		/*ƽ������ */
		printf("average volume is %d\n", (al + ar) /2);
		/*�趨�������������� */
		snd_mixer_selem_set_playback_volume
			(elem, SND_MIXER_SCHN_FRONT_LEFT, 99);
		snd_mixer_selem_set_playback_volume
			(elem, SND_MIXER_SCHN_FRONT_RIGHT, 99);
    }
}
