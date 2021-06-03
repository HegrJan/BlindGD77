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
/* WARNING: THIS FILE USES BINARY (OCTAL) ENCODING, IT USES REMAPPED GLYPHS
 *
 * Translators: SQ7PTE
 *
 * Rev: 4.8
 */
#ifndef USER_INTERFACE_LANGUAGES_POLISH_H_
#define USER_INTERFACE_LANGUAGES_POLISH_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with binary encoding
 *
 ********************************************************************/
const stringsTable_t polishLanguage =
{
.LANGUAGE_NAME 			= "Polski", // MaxLen: 16
.language				= "J�zyk", // MaxLen: 16
.menu					= "Spis", // MaxLen: 16
.credits				= "Wsp�tw�rcy", // MaxLen: 16
.zone					= "Strefy", // MaxLen: 16
.rssi					= "Sygna�", // MaxLen: 16
.battery				= "Bateria", // MaxLen: 16
.contacts				= "Kontakty", // MaxLen: 16
.firmware_info				= "Wersja programu", // MaxLen: 16
.last_heard				= "Ostatnio aktywne", // MaxLen: 16
.options				= "Opcje og�lne", // MaxLen: 16
.display_options			= "Opcje ekranu", // MaxLen: 16
.sound_options				= "Opcje d�wi�ku", // MaxLen: 16
.channel_details			= "Detale kana�u", // MaxLen: 16
.new_contact				= "Nowy kontakt", // MaxLen: 16
.dmr_contacts				= "DMR kontakt", // MaxLen: 16
.hotspot_mode				= "HotSpot", // MaxLen: 16
.contact_details			= "Detale kontaktu", // MaxLen: 16
.built					= "Kompilacja", // MaxLen: 16
.zones					= "Strefy", // MaxLen: 16
.keypad					= "Klawiatura", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "zablokowana", // MaxLen: 15
.press_blue_plus_star			= "Niebieski + *", // MaxLen: 19
.to_unlock				= "aby odblokowa�", // MaxLen: 19
.unlocked				= "Odblokowane", // MaxLen: 15
.power_off				= "Wy��czanie...", // MaxLen: 16
.error					= "B��D", // MaxLen: 8
.rx_only				= "Tylko odbi�r", // MaxLen: 14
.out_of_band				= "POZA PASMEM", // MaxLen: 14
.timeout				= "CZAS MIN��", // MaxLen: 8
.tg_entry				= "Wpisz numer TG", // MaxLen: 15
.pc_entry				= "Wpisz numer PC", // MaxLen: 15
.user_dmr_id				= "ID u�ytkownika", // MaxLen: 15
.contact 				= "Kontakt", // MaxLen: 15
.accept_call				= "Zaakceptowa�?", // MaxLen: 16
.private_call				= "Prywatne", // MaxLen: 16
.squelch				= "Squelch", // MaxLen: 8
.quick_menu 				= "Szybkie opcje", // MaxLen: 16
.filter					= "Filtr", // MaxLen: 7 (with ':' + settings: .none, "CC", "CC,TS", "CC,TS,TG")
.all_channels				= "Wszystkie", // MaxLen: 16
.gotoChannel				= "Id� do",  // MaxLen: 11 (" 1024")
.scan					= "Skanowanie", // MaxLen: 16
.channelToVfo				= "Kana� > VFO", // MaxLen: 16
.vfoToChannel				= "VFO > Kana�", // MaxLen: 16
.vfoToNewChannel		= "VFO > Nowy kana�", // MaxLen: 16
.group					= "Grupa", // MaxLen: 16 (with .type)
.private				= "Prywatny", // MaxLen: 16 (with .type)
.all					= "Wszystko", // MaxLen: 16 (with .type)
.type					= "Typ", // MaxLen: 16 (with .type)
.timeSlot				= "Szczelina", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "Brak", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:", and .filter)
.contact_saved				= "Zapisz kontakt", // MaxLen: 16
.duplicate				= "Duplikat", // MaxLen: 16
.tg					= "TG",  // MaxLen: 8
.pc					= "PC", // MaxLen: 8
.ts					= "TS", // MaxLen: 8
.mode					= "Tryb",  // MaxLen: 12
.colour_code				= "Kolor kodu", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A",// MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "Pasmo", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "Krok", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "Wy�.", // MaxLen: 16 (with ':' + .timeout_beep, .calibration or .band_limits)
.zone_skip				= "Pomi� kana�", // MaxLen: 16 (with ':' + .yes or .no)
.all_skip				= "Pomi� All", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "Tak", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no					= "Nie", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				= "Grupa", // MaxLen: 16 (with ':' and codeplug group name)
.on					= "W�.", // MaxLen: 16 (with ':' + .calibration or .band_limits)
.timeout_beep				= "Czas bipa", // MaxLen: 16 (with ':' + .off or 5..20)
.UNUSED_1				= "",
.calibration				= "Kalibracja", // MaxLen: 16 (with ':' + .on or .off)
.band_limits				= "Limit pasma", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume				= "G�os bipa", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain				= "DMR mic", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM mic", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Key d�ugi", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "Key kr�tki", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout			= "Czas filtra", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "Jasno��", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off				= "Mini jasno��", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "Kontrast", // MaxLen: 16 (with ':' + 12..30)
.colour_invert				= "Inwersja", // MaxLen: 16
.colour_normal				= "Normalny", // MaxLen: 16
.backlight_timeout			= "�wiecenie", // MaxLen: 16 (with ':' + .no to 30)
.scan_delay				= "Czas skan.", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase					= "TAK", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase					= "NIE", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "ODWO�A�", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "Tryb skan.", // MaxLen: 16 (with ':' + .hold or .pause)
.hold					= "St�j", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "Pauza", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "Lista pusta", // MaxLen: 16
.delete_contact_qm			= "Usu� kontakt?", // MaxLen: 16
.contact_deleted			= "Kontakt usuniety", // MaxLen: 16
.contact_used				= "Kontakt istnieje", // MaxLen: 16
.in_rx_group				= "W grupie RX", // MaxLen: 16
.select_tx				= "Wybierz TX", // MaxLen: 16
.edit_contact				= "Edytuj kontakt", // MaxLen: 16
.delete_contact				= "Usu� kontakt", // MaxLen: 16
.group_call				= "Grupy", // MaxLen: 16
.all_call				= "��cze wszystkich", // MaxLen: 16
.tone_scan				= "Skanuj",//// MaxLen: 16
.low_battery				= "S�ABA BATERIA!!!",//// MaxLen: 16
.Auto					= "Automat", // MaxLen 16 (with .mode + ':')
.manual					= "Manualny",  // MaxLen 16 (with .mode + ':') 
.ptt_toggle				= "Sta�e PTT", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling  = "Zezw�l PC", // MaxLen 16 (with ':' + .on ot .off)
.stop					= "Stop", // Maxlen 16 (with ':' + .scan_mode)
.one_line				= "1 linia", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 linie", // MaxLen 16 (with ':' + .contact)
.new_channel				= "Nowy kana�", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "Wyb�r", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "Wyb�r bipa", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "Start", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "Oba", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX Thres.", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX Tail", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "Bip audio",// Maxlen 16 (with ':' + .silent, .normal, .beep or .voice_prompt_level_1)
.silent                                 = "Brak", // Maxlen 16 (with : + audio_prompt)
.normal                                 = "Normal", // Maxlen 16 (with : + audio_prompt)
.beep					= "Bip", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1					= "G�os 1", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias	= "TA Tx", // Maxlen 16 (with : + .on or .off)
.squelch_VHF			= "VHF Squelch",// Maxlen 16 (with : + XX%)
.squelch_220			= "220 Squelch",// Maxlen 16 (with : + XX%)
.squelch_UHF			= "UHF Squelch", // Maxlen 16 (with : + XX%)
.display_background_colour = "Kolor" , // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 			= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git commit",
.voice_prompt_level_2	= "G�os 2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3	= "G�os 3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR Filtr",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.dmr_cc_filter			= "CC Filtr", // MaxLen: 12 (with ':' + settings: .on or .off)
.dmr_ts_filter			= "TS Filtr", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF kontakt", // Maxlen: 16
.channel_power				= "Moc kana�u", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Master",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Set Quick Key", // MaxLen: 16
.dual_watch				= "Nas�uch 1i2 VFO", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or.ts or .pwr or .both)
.pwr					= "Moc",
.user_power				= "Max moc +W-",
.temperature				= "Temperatura", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "sekund",
.radio_info				= "Info o radiu",
.temperature_calibration		= "Temp Cal",
.pin_code				= "Pin Code",
.please_confirm				= "Please confirm", // MaxLen: 15
.vfo_freq_bind_mode			= "Wi�� L1 i L2",
.overwrite_qm				= "Przypisa� ?", //Maxlen: 14 chars
.eco_level				= "Eco poziom",
.buttons				= "Klawisz",
.leds					= "�wiat�o LED",
.scan_dwell_time			= "Scan dwell",
.battery_calibration			= "Batt. Cal",
.low					= "Low",
.high					= "High",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scan On Boot"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with binary encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_POLISH_H_ */
