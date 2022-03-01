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
 * Translators: 9A3HVZ
 *
 *
 * Rev: 08 Jan 2022
 */
#ifndef USER_INTERFACE_LANGUAGES_CROATIAN_H_
#define USER_INTERFACE_LANGUAGES_CROATIAN_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t croatianLanguage =
{
.LANGUAGE_NAME 				= "Hrvatski", // MaxLen: 16
.menu					= "Meni", // MaxLen: 16
.credits				= "Zasluge", // MaxLen: 16
.zone					= "Grupe", // MaxLen: 16
.rssi					= "Razina signala", // MaxLen: 16
.battery				= "Baterija", // MaxLen: 16
.contacts				= "Kontakti", // MaxLen: 16
.last_heard				= "Zadnje cujni", // MaxLen: 16
.firmware_info				= "Firmware verzija", // MaxLen: 16
.options				= "Opcije", // MaxLen	: 16
.display_options			= "Opcije zaslona", // MaxLen: 16
.sound_options				= "Opcije zvuka", // MaxLen: 16
.channel_details			= "Detalji kanala", // MaxLen: 16
.language				= "Jezik", // MaxLen: 16
.new_contact				= "Novi kontakt", // MaxLen: 16
.dmr_contacts				= "DMR kontakti", // MaxLen: 16
.contact_details			= "Detalji kontakta", // MaxLen: 16
.hotspot_mode				= "Hotspot", // MaxLen: 16
.built					= "Stvoreno", // MaxLen: 16
.zones					= "Grupe", // MaxLen: 16
.keypad					= "Tipkovnica", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "Zakljucano", // MaxLen: 15
.press_blue_plus_star			= "Pritisni plavo + *", // MaxLen: 19
.to_unlock				= "Za otkljucati", // MaxLen: 19
.unlocked				= "Otkljucano", // MaxLen: 15
.power_off				= "Iskljuceno", // MaxLen: 16
.error					= "Greska", // MaxLen: 8
.rx_only				= "Samo Rx", // MaxLen: 14
.out_of_band				= "IZVAN BANDA", // MaxLen: 14
.timeout				= "Zauzeto", // MaxLen: 8
.tg_entry				= "TG unos", // MaxLen: 15
.pc_entry				= "PC unos", // MaxLen: 15
.user_dmr_id				= "DMR ID stanice", // MaxLen: 15
.contact 				= "Kontakt", // MaxLen: 15
.accept_call				= "Prihvati poziv", // MaxLen: 16
.private_call				= "Privatni poziv", // MaxLen: 16
.squelch				= "Sum", // MaxLen: 8
.quick_menu 				= "Brzi Meni", // MaxLen: 16
.filter					= "Filteri", // MaxLen: 7 (with ':' + settings: .none, "CC", "CC,TS", "CC,TS,TG")
.all_channels				= "Svi kanali", // MaxLen: 16
.gotoChannel				= "Idi na",  // MaxLen: 11 (" 1024")
.scan					= "Trazi", // MaxLen: 16
.channelToVfo				= "Kanal --> VFO", // MaxLen: 16
.vfoToChannel				= "VFO --> Kanal", // MaxLen: 16
.vfoToNewChannel			= "VFO --> N.kanal", // MaxLen: 16
.group					= "Grupa", // MaxLen: 16 (with .type)
.private				= "Privat", // MaxLen: 16 (with .type)
.all					= "Svi", // MaxLen: 16 (with .type)
.type					= "Tip", // MaxLen: 16 (with .type)
.timeSlot				= "Time slot", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "Nista", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:", .filter/.mode/.dmr_beep)
.contact_saved				= "Kontakt spremi", // MaxLen: 16
.duplicate				= "Duplikat", // MaxLen: 16
.tg					= "TG",  // MaxLen: 8
.pc					= "PC", // MaxLen: 8
.ts					= "TS", // MaxLen: 8
.mode					= "Vrsta",  // MaxLen: 12
.colour_code				= "Color code", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A",// MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "Sirina", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "Korak", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "Isk", // MaxLen: 16 (with ':' + .timeout_beep, .band_limits)
.zone_skip				= "Preskoci zonu", // MaxLen: 16 (with ':' + .yes or .no) 
.all_skip				= "Preskoci sve", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "Da", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no					= "Ne", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				= "Rx Grupa", // MaxLen: 16 (with ':' and codeplug group name)
.on					= "Ukl", // MaxLen: 16 (with ':' + .band_limits)
.timeout_beep				= "Istek zvuka", // MaxLen: 16 (with ':' + .off or 5..20)
.list_full				= "Sva lista",
.UNUSED_1				= "",
.band_limits				= "Limit band", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume				= "Glasnoca", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain				= "DMR mic", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM mic", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Dugo drzi", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "Opet drzi", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout			= "Filter time", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "Svjetlost", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off				= "Min svjetlo", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "Kontrast", // MaxLen: 16 (with ':' + 12..30)
.colour_invert				= "Tamni zaslon", // MaxLen: 16
.colour_normal				= "Svijetli zaslon", // MaxLen: 16
.backlight_timeout			= "Trajanje", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				= "Odgodi sken", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase			= "Da", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase			= "Ne", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "ODBACI", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "Za sken", // MaxLen: 16 (with ':' + .hold, .pause or .stop)
.hold					= "Drzi", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "Pauza", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "Prazna lista", // MaxLen: 16
.delete_contact_qm			= "Obrisi kontakt?", // MaxLen: 16
.contact_deleted			= "Kontakt obrisan", // MaxLen: 16
.contact_used				= "Trazeni kontakt", // MaxLen: 16
.in_rx_group				= "U RX grupi", // MaxLen: 16
.select_tx				= "Odaberi TX", // MaxLen: 16
.edit_contact				= "Uredi kontakt", // MaxLen: 16
.delete_contact				= "Obrisi kontakt", // MaxLen: 16
.group_call				= "Grupni poziv", // MaxLen: 16
.all_call				= "Svi pozivi", // MaxLen: 16
.tone_scan				= "Trazi ton", // MaxLen: 16
.low_battery				= "PRAZNA BATERIJA!", // MaxLen: 16
.Auto					= "Automatsko", // MaxLen 16 (with .mode + ':') 
.manual					= "Rucno",  // MaxLen 16 (with .mode + ':')
.ptt_toggle				= "Drzi PTT", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling			= "Dozvoli PC", // MaxLen 16 (with ':' + .on or .off)
.stop					= "Stop", // Maxlen 16 (with ':' + .scan_mode/.dmr_beep)
.one_line				= "1 red", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 red", // MaxLen 16 (with ':' + .contact)
.new_channel				= "Novi kanal", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "Zaporedje", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "DMR zvuk", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "Start", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "Oba", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX prag", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX rep", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "Zvuk",// Maxlen 16 (with ':' + .silent, .beep or .voice_prompt_level_1)
.silent                                 = "Tiho", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2				= "",
.beep					= "Bip", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1			= "Glas", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias			= "TA Tx", // Maxlen 16 (with : + .on or .off)
.squelch_VHF				= "VHF Sum",// Maxlen 16 (with : + XX%)
.squelch_220				= "220 Sum",// Maxlen 16 (with : + XX%)
.squelch_UHF				= "UHF Sum", // Maxlen 16 (with : + XX%)
.display_background_colour 		= "Boja", // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 				= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git commit",
.voice_prompt_level_2			= "Glas L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3			= "Glas L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR Filter",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.UNUSED_4				= "",
.dmr_ts_filter				= "TS Filter", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF contacts", // Maxlen: 16
.channel_power				= "Ch Snaga", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Master",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Brzo postavi", // MaxLen: 16
.dual_watch				= "Dvostruki pogled", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or .ts or .pwr or .both)
.pwr					= "Pwr",
.user_power				= "Moja snaga",
.temperature				= "Temperatura", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "sekunde",
.radio_info				= "O Radiju",
.temperature_calibration		= "Temp Cal",
.pin_code				= "Pin kod",
.please_confirm				= "Molim potvru", // MaxLen: 15
.vfo_freq_bind_mode			= "Frek. veze",
.overwrite_qm				= "Prenjeti ?", //Maxlen: 14 chars
.eco_level				= "Eko Razina",
.buttons				= "Tipke",
.leds					= "LED",
.scan_dwell_time			= "Rok skena",
.battery_calibration			= "Bat. kali",
.low					= "Slabo",
.high					= "Jako",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Sken Pri Pokr",
.dtmf_entry				= "DTMF unos",
.name					= "Ime",
.UNUSED_3				= "",
.openDM1801A 				= "OpenDM1801A", // Do not translate
.time					= "Vrijeme",
.uptime					= "U radu",
.hours					= "Sati",
.minutes				= "Minute",
.satellite				= "Sateliti",
.alarm_time				= "Alarm time",
.location				= "Lokacija",
.date					= "Datum",
.timeZone				= "UTC zona",
.suspend				= "Obustava",
.pass					= "Prosao", // For satellite screen
.elevation				= "El",
.azimuth				= "Az",
.inHHMMSS				= "u",
.predicting				= "Predviden",
.maximum				= "Max",
.satellite_short			= "Sats",
.local					= "Lokal",
.UTC					= "UTC",
.symbols				= "NSEW", // symbols: N,S,E,W
.not_set				= "NE JOS",
.general_options			= "Sve opcije",
.radio_options				= "Radio opcije"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_CROATIAN_H_ */
