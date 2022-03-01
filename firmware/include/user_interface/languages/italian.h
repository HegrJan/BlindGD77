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
 * Translators: IU4LEG, IZ2EIB
 *
 *
 * Rev: 2021.09.11 IZ2EIB & IU4LEG
 */
#ifndef USER_INTERFACE_LANGUAGES_ITALIAN_H_
#define USER_INTERFACE_LANGUAGES_ITALIAN_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t italianLanguage =
{
.LANGUAGE_NAME 			   = "Italiano", // MaxLen: 16
.menu					   = "Menu", // MaxLen: 16
.credits				   = "Crediti", // MaxLen: 16
.zone					   = "Zone", // MaxLen: 16
.rssi					   = "RSSI", // MaxLen: 16
.battery				   = "Batteria", // MaxLen: 16
.contacts				   = "Contatti", // MaxLen: 16
.last_heard				   = "Ultimi Ricevuti", // MaxLen: 16
.firmware_info			   = "Informazioni", // MaxLen: 16
.options				   = "Opzioni", // MaxLen: 16
.display_options		   = "Opz. Display", // MaxLen: 16
.sound_options			   = "Opzioni Audio", // MaxLen: 16
.channel_details		   = "Dettagli canale", // MaxLen: 16
.language				   = "Lingua", // MaxLen: 16
.new_contact			   = "Nuovo Contatto", // MaxLen: 16
.dmr_contacts				= "DMR contacts", // MaxLen: 16
.contact_details		   = "Det.gli Contatto", // MaxLen: 16
.hotspot_mode			   = "Hotspot", // MaxLen: 16
.built					   = "Versione", // MaxLen: 16
.zones					   = "Zone", // MaxLen: 16
.keypad					   = "Tastiera", // MaxLen: 12 (with .ptt)
.ptt					   = "PTT", // MaxLen: 12 (with .keypad)
.locked					   = "Bloccato", // MaxLen: 15
.press_blue_plus_star	   = "Premi Blue + *", // MaxLen: 19
.to_unlock				   = "Per sbloccare", // MaxLen: 19
.unlocked				   = "Sbloccato", // MaxLen: 15
.power_off				   = "Spegnimento...", // MaxLen: 16
.error					   = "ERRORE", // MaxLen: 8
.rx_only				   = "Solo Rx", // MaxLen: 14
.out_of_band			   = "FUORI BANDA", // MaxLen: 14
.timeout				   = "TIMEOUT", // MaxLen: 8
.tg_entry				   = "Inserisci TG", // MaxLen: 15
.pc_entry				   = "Inserisci CP", // MaxLen: 15
.user_dmr_id			   = "ID Utente DMR", // MaxLen: 15
.contact 				   = "Contatto",// MaxLen: 15
.accept_call			   = "Rispondere a", // MaxLen: 16
.private_call			   = "Chiamata Priv.", // MaxLen: 16
.squelch				   = "Squelch", // MaxLen: 8
.quick_menu 			   = "Menu rapido", // MaxLen: 16
.filter					   = "Filtro", // MaxLen: 7 (with ':' + settings: .none, "CC", "CC,TS", "CC,TS,TG")
.all_channels			   = "Tutti i canali", // MaxLen: 16
.gotoChannel			   = "Vai a", // MaxLen: 11 (" 1024")
.scan					   = "Scansione", // MaxLen: 16
.channelToVfo			   = "Canale --> VFO", // MaxLen: 16
.vfoToChannel			   = "VFO --> Canale", // MaxLen: 16
.vfoToNewChannel		   = "VFO --> Nuovo Ch", // MaxLen: 16
.group					   = "Gruppo", // MaxLen: 16 (with .type)
.private				   = "Privato", // MaxLen: 16 (with .type)
.all					   = "Tutti", // MaxLen: 16 (with .type)
.type					   = "Tipo", // MaxLen: 16 (with .type)
.timeSlot				   = "Timeslot", // MaxLen: 16 (plus ':' and  .none, '1' or '2')
.none					   = "Nessuno", // MaxLen: 16 (with .timeSlot, "Rx CTCSS:" and ""Tx CTCSS:", .filter and .mode )
.contact_saved			   = "Salvato", // MaxLen: 16
.duplicate				   = "Duplicato", // MaxLen: 16
.tg						   = "TG", // MaxLen: 8
.pc						   = "CP", // MaxLen: 8
.ts						   = "TS", // MaxLen: 8
.mode					   = "Modo", // MaxLen: 12
.colour_code			   = "Codice Colore", // MaxLen: 16 (with ':' * .n_a)
.n_a					   = "N/A", // MaxLen: 16 (with ':' * .colour_code)
.bandwidth				   = "Banda", // MaxLen: 16 (with ':' + .n_a, "25kHz" or "12.5kHz")
.stepFreq				   = "Step", // MaxLen: 7 (with ':' + xx.xxkHz fitted)
.tot					   = "TOT", // MaxLen: 16 (with ':' + .off or 15..3825)
.off					   = "Off", // MaxLen: 16 (with ':' + .timeout_beep, .band_limits)
.zone_skip				   = "Salta Zona", // MaxLen: 16 (with ':' + .yes or .no)
.all_skip				   = "Salta Tutti",// MaxLen: 16 (with ':' + .yes or .no)
.yes					   = "S�", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.no						   = "No", // MaxLen: 16 (with ':' + .zone_skip, .all_skip)
.rx_group				   = "Rx Grp", // MaxLen: 16 (with ':' and codeplug group name)
.on						   = "On", // MaxLen: 16 (with ':' + .band_limits)
.timeout_beep			   = "Bip Timeout", // MaxLen: 16 (with ':' + .off or 5..20)
.list_full				= "List full",
.UNUSED_1			= "",
.band_limits			   = "Limiti Banda", // MaxLen: 16 (with ':' + .on or .off)
.beep_volume			   = "Volume Bip", // MaxLen: 16 (with ':' + -24..6 + 'dB')
.dmr_mic_gain			   = "DMR mic", // MaxLen: 16 (with ':' + -33..12 + 'dB')
.fm_mic_gain			   = "FM mic", // MaxLen: 16 (with ':' + 0..31)
.key_long				   = "Key long", // MaxLen: 11 (with ':' + x.xs fitted)
.key_repeat				   = "Key rpt", // MaxLen: 11 (with ':' + x.xs fitted)
.dmr_filter_timeout		   = "Tempo Filtro", // MaxLen: 16 (with ':' + 1..90 + 's')
.brightness				   = "Luminosit�", // MaxLen: 16 (with ':' + 0..100 + '%')
.brightness_off			   = "Min. Lum.", // MaxLen: 16 (with ':' + 0..100 + '%')
.contrast				   = "Contrasto", // MaxLen: 16 (with ':' + 12..30)
.colour_invert			   = "Invert.", // MaxLen: 16
.colour_normal			   = "Normale", // MaxLen: 16
.backlight_timeout		   = "Timeout", // MaxLen: 16 (with ':' + .no to 30s)
.scan_delay				   = "Ritardo Scan", // MaxLen: 16 (with ':' + 1..30 + 's')
.yes___in_uppercase		   = "S�", // MaxLen: 8 (choice above green/red buttons)
.no___in_uppercase		   = "NO", // MaxLen: 8 (choice above green/red buttons)
.DISMISS				   = "RIFIUTA", // MaxLen: 8 (choice above green/red buttons)
.scan_mode				   = "Modo Scan", // MaxLen: 16 (with ':' + .hold, .pause or .stop)
.hold					   = "Blocco", // MaxLen: 16 (with ':' + .scan_mode)
.pause					   = "Pausa", // MaxLen: 16 (with ':' + .scan_mode)
.empty_list				   = "Lista Vuota", // MaxLen: 16
.delete_contact_qm		   = "Canc. Contatto?", // MaxLen: 16
.contact_deleted		   = "Cancellato", // MaxLen: 16
.contact_used			   = "Contatto usato", // MaxLen: 16
.in_rx_group			   = "in gruppo RX", // MaxLen: 16
.select_tx				   = "Seleziona TX", // MaxLen: 16
.edit_contact			   = "Modif. Contatto", // MaxLen: 16
.delete_contact			   = "Canc. Contatto", // MaxLen: 16
.group_call				   = "Chiama Gruppo", // MaxLen: 16
.all_call				   = "Chiama Tutti", // MaxLen: 16
.tone_scan				   = "Scan Toni",//// MaxLen: 16
.low_battery			   = "BATTERIA SCARICA",//// MaxLen: 16
.Auto					   = "Automatico", // MaxLen 16 (with .mode + ':') 
.manual					   = "Manuale",  // MaxLen 16 (with .mode + ':') 
.ptt_toggle				   = "Auto-PTT", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling	   = "Gest. CP", // MaxLen 16 (with ':' + .on ot .off)
.stop					   = "Fine", // Maxlen 16 (with ':' + .scan_mode)
.one_line				   = "1 linea", // MaxLen 16 (with ':' + .contact)
.two_lines				   = "2 linee", // MaxLen 16 (with ':' + .contact)
.new_channel			   = "Nuovo can.", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order			   = "Prio.", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				   = "DMR bip", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					   = "Inizio", // MaxLen 16 (with ':' + .dmr_beep)
.both					   = "Ambedue", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold             = "Soglia VOX", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                  = "Coda VOX", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt			   = "Guida",// Maxlen 16 (with ':' + .silent, .beep or .voice_prompt_level_1)
.silent                    = "Silenziosa", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2			= "",
.beep					   = "Bip", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1	   = "Voce L1", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias	   = "TA Tx", // Maxlen 16 (with : + .on or .off)
.squelch_VHF			   = "VHF Squelch",// Maxlen 16 (with : + XX%)
.squelch_220			   = "220 Squelch",// Maxlen 16 (with : + XX%)
.squelch_UHF			   = "UHF Squelch", // Maxlen 16 (with : + XX%)
.display_background_colour = "Sfondo" , // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				   = "OpenGD77",// Do not translate
.openGD77S 				   = "OpenGD77S",// Do not translate
.openDM1801 			   = "OpenDM1801",// Do not translate
.openRD5R 				   = "OpenRD5R",// Do not translate
.gitCommit				   = "Git commit",
.voice_prompt_level_2	   = "Voce L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3	   = "Voce L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				   = "Filtro DMR",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.UNUSED_4				= "",
.dmr_ts_filter			   = "Filtro TS", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "FM DTMF contacts", // Maxlen: 16
.channel_power				= "W Ch", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "di base",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "Imposta tasti", // MaxLen: 16
.dual_watch				= "Doppio ascolto", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or.ts or .pwr or .both)
.pwr					= "Potenza",
.user_power				= "Volt PA",
.temperature				= "Temperatura", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "secondi",
.radio_info				= "Info radio",
.temperature_calibration		= "Cal Temp",
.pin_code				= "Codice PIN",
.please_confirm				= "Conferma, prego", // MaxLen: 15
.vfo_freq_bind_mode			= "Abbina Freq.",
.overwrite_qm				= "Sovrascrivi ?", //Maxlen: 14 chars
.eco_level				= "Grado consumi",
.buttons				= "Bottoni",
.leds					= "LED",
.scan_dwell_time			= "Ciclo SCAN",
.battery_calibration			= "Cal BATT.",
.low					= "Bassa",
.high					= "Alta",
.dmr_id					= "2�idDMR",
.scan_on_boot				= "Scan su ON",
.dtmf_entry				= "Ins. DTMF",
.name					= "Nome",
.UNUSED_3				= "",
.openDM1801A 				= "OpenDM1801A", // Do not translate
.time					= "Orario",
.uptime					= "Tempo Attivit�",
.hours					= "Ore",
.minutes				= "Minuti",
.satellite				= "Satellite",
.alarm_time				= "Allarme",
.location				= "Posizione",
.date					= "Data",
.timeZone				= "Fuso orario",
.suspend				= "Sospensione",
.pass					= "Pass", // For satellite screen
.elevation				= "El",
.azimuth				= "Az",
.inHHMMSS				= "in",
.predicting				= "Previsione",
.maximum				= "Max",
.satellite_short		= "Sat",
.local					= "Locale",
.UTC					= "UTC",
.symbols				= "NSEO", // symbols: N,S,E,W
.not_set				= "NON IMPOSTATO",
.general_options		= "Opzioni generali",
.radio_options			= "Opzioni radio"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_ITALIAN_H_ */
