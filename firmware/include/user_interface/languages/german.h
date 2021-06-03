/* -*- coding: windows-1252-unix; -*- */
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
 * Translators: DG3GSP, DL4LEX
 *
 *
 * Rev: 4.6
*/
#ifndef USER_INTERFACE_LANGUAGES_GERMAN_H_
#define USER_INTERFACE_LANGUAGES_GERMAN_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t germanLanguage =
{
.LANGUAGE_NAME 			= "Deutsch", // MaxLen: 16
.menu					= "Men�", // MaxLen: 16
.credits				= "Mitwirkende", // MaxLen: 16
.zone					= "Zone", // MaxLen: 16
.rssi					= "Feldst�rke", // MaxLen: 16
.battery				= "Akku", // MaxLen: 16
.contacts				= "Kontakte", // MaxLen: 16
.last_heard				= "Zuletzt geh�rt", // MaxLen: 16
.firmware_info			= "Firmware Info", // MaxLen: 16
.options				= "Einstellungen", // MaxLen: 16
.display_options		= "Display Optionen", // MaxLen: 16
.sound_options				= "Audio Optionen", // MaxLen: 16
.channel_details		= "Kanal Details", // MaxLen: 16
.language				= "Sprache", // MaxLen: 16
.new_contact			= "Neuer Kontakt", // MaxLen: 16
.dmr_contacts				= "DMR Kontakte", // MaxLen: 16
.hotspot_mode			= "Hotspot", // MaxLen: 16
.contact_details		= "Kontakt Details", // MaxLen: 16
.built					= "Erstellt", // MaxLen: 16
.zones					= "Zonen", // MaxLen: 16
.keypad					= "Tasten", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "Gesperrt", // MaxLen: 15
.press_blue_plus_star	= "Blaue Taste + *", // MaxLen: 19
.to_unlock				= "zum entsperren", // MaxLen: 19
.unlocked				= "Entsperrt", // MaxLen: 15
.power_off				= "Schalte aus...", // MaxLen: 16
.error					= "FEHLER", // MaxLen: 8
.rx_only				= "Nur Rx", // MaxLen: 16
.out_of_band			= "AUSSER BAND", // MaxLen: 16
.timeout				= "Zeit abgelaufen", // MaxLen: 8
.tg_entry				= "TG Eingabe", // MaxLen: 15
.pc_entry				= "PC Eingabe", // MaxLen: 15
.user_dmr_id			= "Benutzer ID", // MaxLen: 15
.contact 				= "Kontakt", // MaxLen: 15
.accept_call			= "PC erlauben", // MaxLen: 16
.private_call			= "Privater Ruf", // MaxLen: 16
.squelch				= "Squelch",  // MaxLen: 8
.quick_menu 			= "Schnellfunktion", // MaxLen: 16
.filter					= "Filter", // MaxLen: 7 (with ':' + settings: "None", "CC", "CC,TS", "CC,TS,TG")
.all_channels			= "Alle Kan�le", // MaxLen: 16
.gotoChannel			= "Gehe zu",  // MaxLen: 11 (" 1024")
.scan					= "Suchlauf", // MaxLen: 16
.channelToVfo			= "Kanal --> VFO", // MaxLen: 16
.vfoToChannel			= "VFO --> Kanal", // MaxLen: 16
.vfoToNewChannel		= "VFO --> New Chan", // MaxLen: 16
.group					= "Gruppe", // MaxLen: 16 (with .type)
.private				= "Privat", // MaxLen: 16 (with .type)
.all					= "Alle", // MaxLen: 16 (with .type)
.type					= "Type", // MaxLen: 16 (with .type)
.timeSlot				= "Zeitschlitz", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "Kein", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:")
.contact_saved			= "Kontakt gesp.", // MaxLen: 16
.duplicate				=  "Duplikat", // MaxLen: 16
.tg						= "TG", // MaxLen: 8
.pc						= "PC", // MaxLen: 8
.ts						= "TS", // MaxLen: 8
.mode					= "Modus",  // MaxLen: 12
.colour_code			= "Color Code", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A", // MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "Bandbreite", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "Schritt", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "Aus", // MaxLen: 16 (with ':' + .timeout_beep, .calibration or .band_limits)
.zone_skip				= "Skip Zone", // MaxLen: 16 (with ':' + .yes or .no) 
.all_skip				= "Skip Alle", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "Ja", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no						= "Nein", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				= "Rx Gruppe", // MaxLen: 16 (with ':' and codeplug group name)
.on						= "Ein", // MaxLen: 16 (with ':' + .calibration or .band_limits)
.timeout_beep			= "Timeout-Ton", // MaxLen: 16 (with ':' + .off or 5..20)
.UNUSED_1				= "",
.calibration			= "Kalibration", // MaxLen: 16 (with ':' + .on or .off)
.band_limits			= "Band Limit", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume			= "Beep Lauts", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain			= "DMR Mikro", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM Mikro", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Key lang", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "Key wied", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout		= "DMR Filter", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "Helligkeit", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off			= "Min Helligk.", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "Kontrast", // MaxLen: 16 (with ':' + 12..30)
.colour_invert			= "Invers", // MaxLen: 16
.colour_normal			= "Normal", // MaxLen: 16
.backlight_timeout		= "Timeout", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				= "Scan Verz�g", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase					= "JA", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase						= "NEIN", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "ABLEHNEN", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "Scan Modus", // MaxLen: 16 (with ':' + .hold or .pause)
.hold					= "Halt", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "Pause", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "Leere Liste", // MaxLen: 16
.delete_contact_qm		= "Kontakt l�schen?", // MaxLen: 16
.contact_deleted		= "Kontakt gel�scht", // MaxLen: 16
.contact_used			= "Kontakt benutzt", // MaxLen: 16
.in_rx_group			= "in RX Gruppe", // MaxLen: 16
.select_tx				= "W�hle TX", // MaxLen: 16
.edit_contact			= "Kontakt �ndern", // MaxLen: 16
.delete_contact			= "Kontakt l�schen", // MaxLen: 16
.group_call				= "Gruppenruf", // MaxLen: 16
.all_call				= "Ruf an alle", // MaxLen: 16
.tone_scan				= "CTCSS Scan",//// MaxLen: 16
.low_battery			= "AKKU LEER!",//// MaxLen: 16
.Auto					= "Auto", // MaxLen 16 (with .mode + ':') 
.manual					= "Manuell",  // MaxLen 16 (with .mode + ':') 
.ptt_toggle				= "PTT bistabil", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling		= "Anruf Hinw.", // MaxLen 16 (with ':' + .on ot .off)
.stop					= "Stop", // Maxlen 16 (with ':' + .scan_mode)
.one_line				= "1 Zeile", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 Zeilen", // MaxLen 16 (with ':' + .contact)
.new_channel			= "Neuer Kanal", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "ID-Prio", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "DMR TX Ton", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "Start", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "Beide", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX Empf.", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX Dauer", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "Ansage",// Maxlen 16 (with ':' + .silent, .normal, .beep or .voice_prompt_level_1)
.silent                                 = "Still", // Maxlen 16 (with : + audio_prompt)
.normal                                 = "Normal", // Maxlen 16 (with : + audio_prompt)
.beep					= "T�ne", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1					= "Stimme L1", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias	= "TA senden", // Maxlen 16 (with : + .on or .off)
.squelch_VHF			= "VHF Squelch",// Maxlen 16 (with : + XX%)
.squelch_220			= "220 Squelch",// Maxlen 16 (with : + XX%)
.squelch_UHF			= "UHF Squelch", // Maxlen 16 (with : + XX%)
.display_background_colour = "Anzeige" , // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 			= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git �bergabe",
.voice_prompt_level_2	= "Stimme L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3	= "Stimme L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR Filter",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.dmr_cc_filter			= "CC Filter", // MaxLen: 12 (with ':' + settings: .on or .off)
.dmr_ts_filter			= "TS Filter", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF Kontakte", // Maxlen: 16
.channel_power				= "Ch Leist.", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Master",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Quickkey belegen", // MaxLen: 16
.dual_watch				= "Dual Watch", // MaxLen: 16
.info					= "Infoleiste", // MaxLen: 16 (with ':' + .off or.ts or .pwr or .both)
.pwr					= "Leistung",
.user_power				= "User Power",
.temperature				= "Temperatur", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "sekunden",
.radio_info				= "Radio Infos",
.temperature_calibration		= "Temp Kal",
.pin_code				= "Pin Code",
.please_confirm				= "Bitte best�tigen", // MaxLen: 15
.vfo_freq_bind_mode			= "Freq. Bind",
.overwrite_qm				= "�berschreiben?", //Maxlen: 14 chars
.eco_level				= "Eco Stufe",
.buttons				= "Tasten",
.leds					= "LEDs",
.scan_dwell_time			= "Scan Halt",
.battery_calibration			= "Batt. Cal",
.low					= "Nieder",
.high					= "Hoch",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scan On Boot"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_GERMAN_H  */
