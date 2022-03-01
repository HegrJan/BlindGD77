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
 * Translators: OK2HAD
 *
 *
 * Rev: 4.2
 */
#ifndef USER_INTERFACE_LANGUAGES_CZECH_H_
#define USER_INTERFACE_LANGUAGES_CZECH_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t czechLanguage =
{
.LANGUAGE_NAME 			= "e�tina", // MaxLen: 16
.menu					= "Menu", // MaxLen: 16
.credits				= "P�isp�vatel�", // MaxLen: 16
.zone					= "Z�na", // MaxLen: 16
.rssi					= "S�la sign�lu", // MaxLen: 16
.battery				= "Baterie", // MaxLen: 16
.contacts				= "Kontakty", // MaxLen: 16
.last_heard				= "Posledn�KdoVolal", // MaxLen: 16
.firmware_info			= "Firmware Info", // MaxLen: 16
.options				= "Nastaven�", // MaxLen: 16
.display_options		= "Nastaven�Display", // MaxLen: 16
.sound_options				= "Nastaven� zvuku", // MaxLen: 16
.channel_details		= "Kan�l Detail", // MaxLen: 16
.language				= "Jazyk", // MaxLen: 16
.new_contact			= "Nov� Kontakt", // MaxLen: 16
.dmr_contacts				= "DMR Kontakt", // MaxLen: 16
.contact_details		= "Kontakt Detail", // MaxLen: 16
.hotspot_mode			= "Hotspot-M�d", // MaxLen: 16
.built					= "Sestaven�", // MaxLen: 16
.zones					= "Z�ny", // MaxLen: 16
.keypad					= "Kl�vesa", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "Zam�en�", // MaxLen: 15
.press_blue_plus_star	= "Stla� Modr� + *", // MaxLen: 19
.to_unlock				= "Odemknout", // MaxLen: 19
.unlocked				= "Odemknuto", // MaxLen: 15
.power_off				= "Vyp�nan�...", // MaxLen: 16
.error					= "CHYBA", // MaxLen: 8
.rx_only				= "Jenom Rx", // MaxLen: 16
.out_of_band			= "MIMO P�SMO", // MaxLen: 16
.timeout				= "asLimit", // MaxLen: 8
.tg_entry				= "TG Zad�n�", // MaxLen: 15
.pc_entry				= "PC Zad�n�", // MaxLen: 15
.user_dmr_id			= "U�iv. DMR ID", // MaxLen: 15
.contact 				= "Kontakty", // MaxLen: 15
.accept_call			= "P�ijmout hovor?", // MaxLen: 16
.private_call			= "Seznam kontakt�", // MaxLen: 16
.squelch				= "Squelch",  // MaxLen: 8
.quick_menu 			= "Rychl� menu", // MaxLen: 16
.filter					= "Filtr", // MaxLen: 7 (with ':' + settings: "None", "CC", "CC,TS", "CC,TS,TG")
.all_channels			= "V�echny Kan�ly", // MaxLen: 16
.gotoChannel			= "Dal�� kan�l",  // MaxLen: 11 (" 1024")
.scan					= "Sken", // MaxLen: 16
.channelToVfo			= "Kan�l-->VFO", // MaxLen: 16
.vfoToChannel			= "VFO-->Kan�l", // MaxLen: 16
.vfoToNewChannel		= "VFO-->Nov� Kan�l", // MaxLen: 16
.group					= "Group", // MaxLen: 16 (with .type)
.private				= "Soukrom�", // MaxLen: 16 (with .type)
.all					= "V�echno", // MaxLen: 16 (with .type)
.type					= "Typ", // MaxLen: 16 (with .type)
.timeSlot				= "TimeSlot", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "Nen�", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:")
.contact_saved			= "Kontakt ulo�en", // MaxLen: 16
.duplicate				=  "Duplik�t", // MaxLen: 16
.tg						= "TG", // MaxLen: 8
.pc						= "PC", // MaxLen: 8
.ts						= "TS", // MaxLen: 8
.mode					= "M�d",  // MaxLen: 12
.colour_code			= "Color Code", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A", // MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "��.P�sma", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "Krok", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "Vyp", // MaxLen: 16 (with ':' + .timeout_beep, .band_limits)
.zone_skip				= "Skip v Z�n�", // MaxLen: 16 (with ':' + .yes or .no) 
.all_skip				= "Skip ve V�e", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "Ano", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no						= "Ne", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				= "Rx Group", // MaxLen: 16 (with ':' and codeplug group name)
.on						= "Zap", // MaxLen: 16 (with ':' + .band_limits)
.timeout_beep			= "T�nLimit", // MaxLen: 16 (with ':' + .off or 5..20)
.list_full				= "List full",
.UNUSED_1			= "",
.band_limits			= "OmezitP�smo", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume			= "ZvukKl�ves", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain			= "DMR mic", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM mic", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Dr�et Kl�v.", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "Znovu Stla�", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout		= "asDMRFiltru", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "Jas", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off			= "Min. Jas", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "Kontrast", // MaxLen: 16 (with ':' + 12..30)
.colour_invert			= "ern�", // MaxLen: 16
.colour_normal			= "B�l�", // MaxLen: 16
.backlight_timeout		= "as Sv�cen�", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				= "Sken pauza", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase					= "ANO", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase						= "NE", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "ODM�TNI", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "SkenM�d", // MaxLen: 16 (with ':' + .hold or .pause)
.hold					= "Zastavit", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "Pauza", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "Pr�zdn� seznam", // MaxLen: 16
.delete_contact_qm		= "Smazat kontakt?", // MaxLen: 16
.contact_deleted		= "Kontakt smaz�n", // MaxLen: 16
.contact_used			= "Pou��t kontakt", // MaxLen: 16
.in_rx_group			= "v RX Group", // MaxLen: 16
.select_tx				= "Vybrat TX", // MaxLen: 16
.edit_contact			= "Upravit kontakt", // MaxLen: 16
.delete_contact			= "Smazat Kontakt", // MaxLen: 16
.group_call				= "Seznam Group", // MaxLen: 16
.all_call				= "V�echny �sla", // MaxLen: 16
.tone_scan				= "CTCSS Sken",//// MaxLen: 16
.low_battery			= "SLAB� BATERIE!",//// MaxLen: 16
.Auto					= "Auto", // MaxLen 16 (with .mode + ':') 
.manual					= "Manualn�",  // MaxLen 16 (with .mode + ':') 
.ptt_toggle				= "PTT p�epnout", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling		= "Soukr.Vol�n�", // MaxLen 16 (with ':' + .on ot .off)
.stop					= "stop", // Maxlen 16 (with ':' + .scan_mode)
.one_line				= "1 ��dek", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 ��dky", // MaxLen 16 (with ':' + .contact)
.new_channel			= "Nov� Kan�l", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "�st ID", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "p�pDMR", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "start", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "StartStop", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX Pr�h", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX Dozvuk", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "V�zvaZvuk",// Maxlen 16 (with ':' + .silent, .beep or .voice_prompt_level_1)
.silent                                 = "Tich�", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2			= "",
.beep					= "P�p�n�", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1					= "Hlas-1", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias	= "Vys�lat TA", // Maxlen 16 (with : + .on or .off)
.squelch_VHF			= "VHF Squelch",// Maxlen 16 (with : + XX%)
.squelch_220			= "220 Squelch",// Maxlen 16 (with : + XX%)
.squelch_UHF			= "UHF Squelch", // Maxlen 16 (with : + XX%)
.display_background_colour = "Barva" , // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 			= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git p�ipojen�",
.voice_prompt_level_2	= "Hlas-2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3	= "Hlas-3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR Filtr",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.UNUSED_4				= "",
.dmr_ts_filter			= "TS Filtr", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF kontakt", // Maxlen: 16
.channel_power				= "V�konKan�l", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Ru�n�",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Nast.Modr� + 1-9", // MaxLen: 16
.dual_watch				= "Dual Watch", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or.ts or .pwr or .both)
.pwr					= "V�kon",
.user_power				= "U�iv.V�kon",
.temperature				= "Teplota", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "sekund",
.radio_info				= "R�dio info",
.temperature_calibration		= "Tepl.Kal",
.pin_code				= "K�d PIN",
.please_confirm				= "Pros�m potvrte", // MaxLen: 15
.vfo_freq_bind_mode			= "Freq. Bind",
.overwrite_qm				= "P�epsat ?", //Maxlen: 14 chars
.eco_level				= "Eco Level",
.buttons				= "Buttons",
.leds					= "LEDs",
.scan_dwell_time			= "Scan dwell",
.battery_calibration			= "Batt. Cal",
.low					= "Low",
.high					= "High",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scan On Boot",
.dtmf_entry				= "DTMF entry",
.name					= "Name",
.UNUSED_3				= "",
.openDM1801A 				= "OpenDM1801A", // Do not translate
.time					= "Time",
.uptime					= "Uptime",
.hours					= "Hours",
.minutes				= "Minutes",
.satellite				= "Satellite",
.alarm_time				= "Alarm time",
.location				= "Location",
.date					= "Date",
.timeZone				= "Timezone",
.suspend				= "Suspend",
.pass					= "Pass", // For satellite screen
.elevation				= "El",
.azimuth				= "Az",
.inHHMMSS				= "in",
.predicting				= "Predicting",
.maximum				= "Max",
.satellite_short		= "Sat",
.local					= "Local",
.UTC					= "UTC",
.symbols				= "NSEW", // symbols: N,S,E,W
.not_set				= "NOT SET",
.general_options		= "General options",
.radio_options			= "Radio options"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_CZECH_H  */
