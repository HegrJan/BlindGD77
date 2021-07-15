/* -*- coding: binary; -*- */
/* 
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. Use of this source code or binary releases for commercial purposes is strictly forbidden. This includes, without limitation,
 *    incorporation in a commercial product or incorporation into a product or project which allows commercial use.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Translators: JE4SMQ
 *
 *
 * Rev:
 */
#ifndef USER_INTERFACE_LANGUAGES_JAPANESE_H_
#define USER_INTERFACE_LANGUAGES_JAPANESE_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t japaneseLanguage =
{
.LANGUAGE_NAME 				= "��ݺ�", // MaxLen: 16
.menu					= "�ƭ-", // MaxLen: 16
.credits				= "���¼�", // MaxLen: 16
.zone					= "��-�", // MaxLen: 16
.rssi					= "RSSI", // MaxLen: 16
.battery				= "����", // MaxLen: 16
.contacts				= "�����", // MaxLen: 16
.last_heard				= "�ޭ��۸�", // MaxLen: 16
.firmware_info				= "̧-ѳ���ޮ�γ", // MaxLen: 16
.options				= "��߼��", // MaxLen: 16
.display_options			= "ˮ��� ��߼��", // MaxLen: 16
.sound_options				= "�ݾ�  ��߼��", // MaxLen: 16
.channel_details			= "����� Ųֳ", // MaxLen: 16
.language				= "Language", // MaxLen: 16
.new_contact				= "New �����", // MaxLen: 16
.dmr_contacts				= "DMR �����", // MaxLen: 16
.contact_details			= "�����Ųֳ", // MaxLen: 16
.hotspot_mode				= "ίĽ�߯�", // MaxLen: 16
.built					= "Built", // MaxLen: 16
.zones					= "��-�", // MaxLen: 16
.keypad					= "�-�߯��", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "ۯ�", // MaxLen: 15
.press_blue_plus_star			= "F(��) + *", // MaxLen: 19
.to_unlock				= "ۯ����ޮ", // MaxLen: 19
.unlocked				= "ۯ����ޮ���", // MaxLen: 15
.power_off				= "��ݹ�� Off...", // MaxLen: 16
.error					= "��-", // MaxLen: 8
.rx_only				= "���ݷݼ", // MaxLen: 14
.out_of_band				= "�������", // MaxLen: 14
.timeout				= "��ѱ��", // MaxLen: 8
.tg_entry				= "TG ƭ�خ�", // MaxLen: 15
.pc_entry				= "PC ƭ�خ�", // MaxLen: 15
.user_dmr_id				= "�-��- DMR ID", // MaxLen: 15
.contact 				= "�����", // MaxLen: 15
.accept_call				= "Return call to", // MaxLen: 16
.private_call				= "��ײ��-ĺ-�", // MaxLen: 16
.squelch				= "����", // MaxLen: 8
.quick_menu 				= "�����ƭ-", // MaxLen: 16
.filter					= "̨��-", // MaxLen: 7 (with ':' + settings: .none, "CC", "CC,TS", "CC,TS,TG")
.all_channels				= "��������", // MaxLen: 16
.gotoChannel				= "����ٲ�޳",  // MaxLen: 11 (" 1024")
.scan					= "����", // MaxLen: 16
.channelToVfo				= "����� --> VFO", // MaxLen: 16
.vfoToChannel				= "VFO --> �����", // MaxLen: 16
.vfoToNewChannel			= "VFO --> New�����", // MaxLen: 16
.group					= "���-��", // MaxLen: 16 (with .type)
.private				= "��ײ��-�", // MaxLen: 16 (with .type)
.all					= "�-�", // MaxLen: 16 (with .type)
.type					= "����", // MaxLen: 16 (with .type)
.timeSlot				= "��ѽۯ�", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "ż", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:", .filter/.mode/.dmr_beep)
.contact_saved				= "����� ο�ݽ�", // MaxLen: 16
.duplicate				= "�ޭ�̸", // MaxLen: 16
.tg					= "TG",  // MaxLen: 8
.pc					= "PC", // MaxLen: 8
.ts					= "TS", // MaxLen: 8
.mode					= "�-��",  // MaxLen: 12
.colour_code				= "��-�-��", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A",// MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "��������", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "�ï��", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "��", // MaxLen: 16 (with ':' + .timeout_beep, .band_limits)
.zone_skip				= "��-� �����", // MaxLen: 16 (with ':' + .yes or .no) 
.all_skip				= "�-� �����", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "ʲ", // MaxLen: 16 (with ':' + .zone_skip, .all_skip or .factory_reset)
.no					= "���", // MaxLen: 16 (with ':' + .zone_skip, .all_skip or .factory_reset)
.rx_group				= "RX Grp", // MaxLen: 16 (with ':' and codeplug group name)
.on					= "��", // MaxLen: 16 (with ':' + .band_limits)
.timeout_beep				= "��ѱ����-��", // MaxLen: 16 (with ':' + .off or 5..20)
.list_full				= "List full",
.UNUSED_1			= "",
.band_limits				= "����޾����", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume				= "��-�ߵ�خ�", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain				= "DMR ϲ��޲�", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM ϲ��޲�", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "�-�ݸ�", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "�-���-�", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout			= "̨��-TO", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "��ٻ", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off				= "�ި�-", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "���׽�", // MaxLen: 16 (with ':' + 12..30)
.colour_invert				= "����", // MaxLen: 16
.colour_normal				= "³�ޮ�", // MaxLen: 16
.backlight_timeout			= "��ѱ��", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				= "�����ިڲ", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase			= "ʲ", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase			= "���", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "���߲", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "�����-��", // MaxLen: 16 (with ':' + .hold, .pause or .stop)
.hold					= "�-���", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "��-��", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "ؽ�ż", // MaxLen: 16
.delete_contact_qm			= "�����ɻ��ޮ?", // MaxLen: 16
.contact_deleted			= "����Ļ��ޮ��", // MaxLen: 16
.contact_used				= "�����ʽ��Ʊ�", // MaxLen: 16
.in_rx_group				= "RX���-��Ų", // MaxLen: 16
.select_tx				= "���ݾ���", // MaxLen: 16
.edit_contact				= "����� �����", // MaxLen: 16
.delete_contact				= "����� ���ޮ", // MaxLen: 16
.group_call				= "���-�ߺ-�", // MaxLen: 16
.all_call				= "�-ٺ-�", // MaxLen: 16
.tone_scan				= "�-ݽ���", // MaxLen: 16
.low_battery				= "����-��� !!!", // MaxLen: 16
.Auto					= "�-�", // MaxLen 16 (with .mode + ':') 
.manual					= "�ƭ��",  // MaxLen 16 (with .mode + ':')
.ptt_toggle				= "PTT ĸ��", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling			= "PC��¹", // MaxLen 16 (with ':' + .on or .off)
.stop					= "�į��", // Maxlen 16 (with ':' + .scan_mode/.dmr_beep)
.one_line				= "1 �ޮ�", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 �ޮ�", // MaxLen 16 (with ':' + .contact)
.new_channel				= "New �����", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "DB�ޭ�", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "DMR ��-��", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "��-�", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "خ�γ", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX �گ�����", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX �-�", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "�ݾ���Ų",// Maxlen 16 (with ':' + .silent, .beep or .voice_prompt_level_1)
.silent                                 = "ż", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2			= "",
.beep					= "��-��", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1			= "�ݾ� L1", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias			= "TA����", // Maxlen 16 (with : + .on or .off)
.squelch_VHF				= "VHF ����",// Maxlen 16 (with : + XX%)
.squelch_220				= "220 ����",// Maxlen 16 (with : + XX%)
.squelch_UHF				= "UHF ����", // Maxlen 16 (with : + XX%)
.display_background_colour 		= "�ޯ���-", // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 				= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git commit",
.voice_prompt_level_2			= "�ݾ� L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3			= "�ݾ� L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR ̨��-",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.dmr_cc_filter				= "CC ̨��-", // MaxLen: 12 (with ':' + settings: .on or .off)
.dmr_ts_filter				= "TS ̨��-", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "DTMF �����ؽ�", // Maxlen: 16
.channel_power				= "Ch Pwr", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Ͻ�-",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "�����- ���", // MaxLen: 16
.dual_watch				= "�ޭ��ܯ�", // MaxLen: 16
.info					= "�ޮ�γ", // MaxLen: 16 (with ':' + .off or .ts or .pwr or .both)
.pwr					= "Pwr",
.user_power				= "�-��-Pwr",
.temperature				= "����", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "�ޮ�",
.radio_info				= "Ѿݷ �ޮ�γ",
.temperature_calibration		= "����Cal",
.pin_code				= "�ݼ����ݺ޳",
.please_confirm				= "���ݼø�޻�", // MaxLen: 15
.vfo_freq_bind_mode			= "TRF���޳",
.overwrite_qm				= "����OK?", //Maxlen: 14 chars
.eco_level				= "������",
.buttons				= "����",
.leds					= "LEDs",
.scan_dwell_time			= "����ò��޶�",
.battery_calibration			= "���������",
.low					= "���",
.high					= "�Բ",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scan On Boot",
.dtmf_entry				= "DTMF entry",
.name					= "Name",
.openDM1801A 				= "OpenDM1801A" // Do not translate
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_JAPANESE_H_ */
