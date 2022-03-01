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
 * Translators: Roger Caballero Jonsson, SM0LTV
 *
 *
 * Rev: 1.2
 */
#ifndef USER_INTERFACE_LANGUAGES_SWEDISH_H_
#define USER_INTERFACE_LANGUAGES_SWEDISH_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t swedishLanguage =
{
.LANGUAGE_NAME 				= "Svenska", // MaxLen: 16
.menu					= "Meny", // MaxLen: 16
.credits				= "Po�ng", // MaxLen: 16
.zone					= "Zon", // MaxLen: 16
.rssi					= "F�ltstyrka", // MaxLen: 16
.battery				= "Batteri", // MaxLen: 16
.contacts				= "Kontakter", // MaxLen: 16
.last_heard				= "Senast h�rd", // MaxLen: 16
.firmware_info				= "Firmware info", // MaxLen: 16
.options				= "Alternativ", // MaxLen: 16
.display_options			= "Visnings alternativ", // MaxLen: 16
.sound_options				= "Ljud alternativ", // MaxLen: 16
.channel_details			= "Kanal detaljer", // MaxLen: 16
.language				= "Spr�k", // MaxLen: 16
.new_contact				= "Ny kontakt", // MaxLen: 16
.dmr_contacts				= "DMR kontakt", // MaxLen: 16
.contact_details			= "Kontakt detaljer", // MaxLen: 16
.hotspot_mode				= "Hotspot", // MaxLen: 16
.built					= "Skapad", // MaxLen: 16
.zones					= "Zoner", // MaxLen: 16
.keypad					= "", // MaxLen: 12 (with .ptt)
.ptt					= "PTT", // MaxLen: 12 (with .keypad)
.locked					= "L�st", // MaxLen: 15
.press_blue_plus_star			= "Tryck Bl� + *", // MaxLen: 19
.to_unlock				= "f�r att l�sa upp", // MaxLen: 19
.unlocked				= "Uppl�st", // MaxLen: 15
.power_off				= "St�nger av...", // MaxLen: 16
.error					= "FEL", // MaxLen: 8
.rx_only				= "Endast Rx", // MaxLen: 14
.out_of_band				= "UTANF�R BANDET", // MaxLen: 14
.timeout				= "TIDSGR.", // MaxLen: 8
.tg_entry				= "TG post", // MaxLen: 15
.pc_entry				= "PC post", // MaxLen: 15
.user_dmr_id				= "Anv�ndar DMR ID", // MaxLen: 15
.contact 				= "Kontakt", // MaxLen: 15
.accept_call				= "Acceptera anrop", // MaxLen: 16
.private_call				= "Privatanrop", // MaxLen: 16
.squelch				= "Brussp.", // MaxLen: 8
.quick_menu 				= "Snabb Menu", // MaxLen: 16
.filter					= "Filter", // MaxLen: 7 (with ':' + settings: .none, "CC", "CC,TS", "CC,TS,TG")
.all_channels				= "Alla kanaler", // MaxLen: 16
.gotoChannel				= "G� till",  // MaxLen: 11 (" 1024")
.scan					= "Skanna", // MaxLen: 16
.channelToVfo				= "Kanal --> VFO", // MaxLen: 16
.vfoToChannel				= "VFO --> Kanal", // MaxLen: 16
.vfoToNewChannel			= "VFO --> Ny kanal", // MaxLen: 16
.group					= "Grupp", // MaxLen: 16 (with .type)
.private				= "Privat", // MaxLen: 16 (with .type)
.all					= "Alla", // MaxLen: 16 (with .type)
.type					= "Typ", // MaxLen: 16 (with .type)
.timeSlot				= "Timeslot", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					= "Ingen", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:", .filter/.mode/.dmr_beep)
.contact_saved				= "Kontakt sparad", // MaxLen: 16
.duplicate				= "Dubletter", // MaxLen: 16
.tg					= "TG",  // MaxLen: 8
.pc					= "PC", // MaxLen: 8
.ts					= "TS", // MaxLen: 8
.mode					= "L�ge",  // MaxLen: 12
.colour_code				= "Color Code", // MaxLen: 16 (with ':' * .n_a)
.n_a					= "N/A",// MaxLen: 16 (with ':' * .colour_code)
.bandwidth				= "Bandbredd", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				= "Steg", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					= "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					= "Av", // MaxLen: 16 (with ':' + .timeout_beep, .calibration or .band_limits)
.zone_skip				= "Zon hopp", // MaxLen: 16 (with ':' + .yes or .no) 
.all_skip				= "All hopp", // MaxLen: 16 (with ':' + .yes or .no)
.yes					= "Ja", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no					= "N", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				= "Rx Grp", // MaxLen: 16 (with ':' and codeplug group name)
.on					= "P�", // MaxLen: 16 (with ':' + .calibration or .band_limits)
.timeout_beep				= "Tidsgr�ns pip", // MaxLen: 16 (with ':' + .off or 5..20)
.list_full				= "List full", ///<<< TRANSLATE
.UNUSED_1				= "",
.band_limits				= "Bandgr�ns", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume				= "Pip vol", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain				= "DMR mik", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain				= "FM mik", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Tng lng", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				= "Tng rpt", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout			= "Filter tid", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				= "Ljusstyrka", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off				= "Min ljus", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				= "Kontrast", // MaxLen: 16 (with ':' + 12..30)
.colour_invert				= "Invert", // MaxLen: 16
.colour_normal				= "Normal", // MaxLen: 16
.backlight_timeout			= "Tidsgr�ns", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				= "Scannf�rdr.", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase			= "JA", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase			= "NEJ", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				= "AVF�RDA", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				= "Scannl�ge", // MaxLen: 16 (with ':' + .hold, .pause or .stop)
.hold					= "H�ll", // MaxLen: 16 (with ':' + .scan_mode)
.pause					= "Paus", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				= "Tom Lista", // MaxLen: 16
.delete_contact_qm			= "Radera kontakt?", // MaxLen: 16
.contact_deleted			= "Kontakt raderad", // MaxLen: 16
.contact_used				= "Kontakten anv�nd", // MaxLen: 16
.in_rx_group				= "i RX grup", // MaxLen: 16
.select_tx				= "V�lj TX", // MaxLen: 16
.edit_contact				= "Redigera kontakt", // MaxLen: 16
.delete_contact				= "Radera kontakt", // MaxLen: 16
.group_call				= "Gruppanrop", // MaxLen: 16
.all_call				= "Allanrop", // MaxLen: 16
.tone_scan				= "Ton scann", // MaxLen: 16
.low_battery				= "L�G BATTERINIV�", // MaxLen: 16
.Auto					= "Auto", // MaxLen 16 (with .mode + ':') 
.manual					= "Manuell",  // MaxLen 16 (with .mode + ':')
.ptt_toggle				= "PTT sp�rr", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling			= "Till�t PC", // MaxLen 16 (with ':' + .on or .off)
.stop					= "Stopp", // Maxlen 16 (with ':' + .scan_mode/.dmr_beep)
.one_line				= "1 linje", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 linjer", // MaxLen 16 (with ':' + .contact)
.new_channel				= "Ny kanal", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "Best�ll", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "DMR pip", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "Start", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "B�da", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "VOX Tr�skel", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "VOX Sl�pp", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "Prompt",// Maxlen 16 (with ':' + .silent, .normal, .beep or .voice_prompt_level_1)
.silent                                 = "Tyst", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2			= "",
.beep					= "Pip", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1			= "R�st", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias			= "TA Tx", // Maxlen 16 (with : + .on or .off)
.squelch_VHF				= "VHF Brussp.",// Maxlen 16 (with : + XX%)
.squelch_220				= "220 Brussp.",// Maxlen 16 (with : + XX%)
.squelch_UHF				= "UHF Brussp.", // Maxlen 16 (with : + XX%)
.display_background_colour 		= "BG F�rg", // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 				= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Git commit",
.voice_prompt_level_2			= "R�st L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3			= "R�st L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "DMR Filter",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.UNUSED_4				= "",
.dmr_ts_filter				= "TS Filter", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF kontakter", // Maxlen: 16
.channel_power				= "Ch Effekt", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Master",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Set Snabbtangent", // MaxLen: 16
.dual_watch				= "Dubbel vakt", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or .ts or .pwr or .both)
.pwr					= "Eff.",
.user_power				= "Anv�ndar Endast",
.temperature				= "Temperatur", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "C",
.seconds				= "sekunder",
.radio_info				= "Radio info",
.temperature_calibration		= "Temp Kal",
.pin_code				= "Pin-kod",
.please_confirm				= "Bekr�fta", // MaxLen: 15
.vfo_freq_bind_mode			= "Bind Freq.",
.overwrite_qm				= "Skriv�ver ?", //Maxlen: 14 chars
.eco_level				= "Eko Niv�",
.buttons				= "Knappar",
.leds					= "LEDs",
.scan_dwell_time			= "Scannstopp",
.battery_calibration			= "Batt. Kal",
.low					= "L�g",
.high					= "H�g",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scanna vid Boot",
.dtmf_entry				= "DTMF post",
.name					= "Namn",
.UNUSED_3				= "",
.openDM1801A 				= "OpenDM1801A", // Do not translate
.time					= "Tid",
.uptime					= "Upetid",
.hours					= "Timmar",
.minutes				= "Minuter",
.satellite				= "Satelit",
.alarm_time				= "Alarm tid",
.location				= "Plats",
.date					= "Datum",
.timeZone				= "Tidezon",
.suspend				= "H�nga",
.pass					= "Pasera",
.elevation				= "El",
.azimuth				= "Az",
.inHHMMSS				= "in",
.predicting				= "F�ruts�ga",
.maximum				= "Max",
.satellite_short			= "Sat",
.local					= "Lokal",
.UTC					= "UTC",
.symbols				= "NS�V", // symbols: N,S,E,W
.not_set				= "EJ INST�LLD",
.general_options			= "General options",
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
#endif /* USER_INTERFACE_LANGUAGES_SWEDISH_H_ */
