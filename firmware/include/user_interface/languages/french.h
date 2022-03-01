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
 * Translators: F1CXG, F1RMB
 *
 *
 * Rev: 3
 */
#ifndef USER_INTERFACE_LANGUAGES_FRENCH_H_
#define USER_INTERFACE_LANGUAGES_FRENCH_H_
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
const stringsTable_t frenchLanguage =
{
.LANGUAGE_NAME			= "Fran�ais",
.menu					= "Menu",
.credits				= "Cr�dits",
.zone					= "Zone",
.rssi					= "RSSI",
.battery				= "Batterie",
.contacts				= "Contacts",
.last_heard				= "Derniers re�us",
.firmware_info				= "Info Firmware",
.options				= "Options",
.display_options			= "Affichage",
.sound_options				= "Audio", // MaxLen: 16
.channel_details			= "D�tails canal",
.language				= "Langue",
.new_contact				= "Nouv. contact",
.dmr_contacts				= "Contacts DMR", // MaxLen: 16
.contact_details			= "D�tails contact",
.hotspot_mode				= "Hotspot",
.built					= "Cr��",
.zones					= "Zones",
.keypad					= "Clavier",
.ptt					= "PTT",
.locked					= "Verrouill�",
.press_blue_plus_star			= "Pressez Bleu + *",
.to_unlock				= "pour d�verrouiller",
.unlocked				= "D�verrouill�",
.power_off				= "Extinction...",
.error					= "ERREUR",
.rx_only				= "Rx Uniqmnt.",
.out_of_band				= "HORS BANDE",
.timeout				= "TIMEOUT",
.tg_entry				= "Entrez TG",
.pc_entry				= "Entrez PC",
.user_dmr_id				= "DMR ID Perso.",
.contact 				= "Contact",
.accept_call				= "R�pondre �",
.private_call				= "Appel Priv�",
.squelch				= "Squelch",
.quick_menu 				= "Menu Rapide",
.filter					= "Filtre",
.all_channels				= "Tous Canaux",
.gotoChannel				= "Aller",
.scan					= "Scan",
.channelToVfo				= "Canal --> VFO",
.vfoToChannel				= "VFO --> Canal",
.vfoToNewChannel			= "VFO --> Nv. Can.", // MaxLen: 16
.group					= "Groupe",
.private				= "Priv�",
.all					= "Tous",
.type					= "Type",
.timeSlot				= "Timeslot",
.none					= "Aucun",
.contact_saved				= "Contact sauv�",
.duplicate				= "Dupliqu�",
.tg					= "TG",
.pc					= "PC",
.ts					= "TS",
.mode					= "Mode",
.colour_code				= "Code Couleur",
.n_a					= "ND",
.bandwidth				= "Larg. bde",
.stepFreq				= "Pas",
.tot					= "TOT",
.off					= "Off",
.zone_skip				= "Saut Zone",
.all_skip				= "Saut Compl.",
.yes					= "Oui",
.no					= "Non",
.rx_group				= "Grp Rx",
.on					= "On",
.timeout_beep				= "Son timeout",
.list_full				= "Liste pleine",
.UNUSED_1			= "",
.band_limits				= "Lim. Bandes",
.beep_volume				= "Vol. bip",
.dmr_mic_gain				= "DMR mic",
.fm_mic_gain				= "FM mic", // MaxLen: 16 (with ':' + 0..31)
.key_long				= "Appui long",
.key_repeat				= "R�p�t�",
.dmr_filter_timeout			= "Tps filtre",
.brightness				= "R�tro �cl.",
.brightness_off				= "�cl. min",
.contrast				= "Contraste",
.colour_invert				= "Inverse",
.colour_normal				= "Normale",
.backlight_timeout			= "Timeout",
.scan_delay				= "D�lai scan",
.yes___in_uppercase			= "OUI",
.no___in_uppercase			= "NON",
.DISMISS				= "CACHER",
.scan_mode				= "Mode scan",
.hold					= "Bloque",
.pause					= "Pause",
.empty_list				= "Liste Vide",
.delete_contact_qm			= "Efface contact ?",
.contact_deleted			= "Contact effac�",
.contact_used				= "Contact utilis�",
.in_rx_group				= "ds le groupe RX",
.select_tx				= "Select� TX",
.edit_contact				= "�dite Contact",
.delete_contact				= "Efface Contact",
.group_call				= "Appel de Groupe",
.all_call				= "All Call",
.tone_scan				= "Scan tons",
.low_battery			        = "BATT. FAIBLE !!!",//// MaxLen: 16
.Auto					= "Auto", // MaxLen 16 (with .mode + ':') 
.manual					= "Manuel",  // MaxLen 16 (with .mode + ':') }
.ptt_toggle				= "Bascule PTT", // MaxLen 16 (with ':' + .on or .off)
.private_call_handling			= "Filtre PC", // MaxLen 16 (with ':' + .on ot .off)
.stop					= "Arr�t", // Maxlen 16 (with ':' + .scan_mode)
.one_line				= "1 ligne", // MaxLen 16 (with ':' + .contact)
.two_lines				= "2 lignes", // MaxLen 16 (with ':' + .contact)
.new_channel				= "Nouv. canal", // MaxLen: 16, leave room for a space and four channel digits after
.priority_order				= "Ordre", // MaxLen 16 (with ':' + 'Cc/DB/TA')
.dmr_beep				= "Bip TX", // MaxLen 16 (with ':' + .star/.stop/.both/.none)
.start					= "D�but", // MaxLen 16 (with ':' + .dmr_beep)
.both					= "Les Deux", // MaxLen 16 (with ':' + .dmr_beep)
.vox_threshold                          = "Seuil VOX", // MaxLen 16 (with ':' + .off or 1..30)
.vox_tail                               = "Queue VOX", // MaxLen 16 (with ':' + .n_a or '0.0s')
.audio_prompt				= "Prompt",// Maxlen 16 (with ':' + .silent, .beep or .voice_prompt_level_1)
.silent                                 = "Silence", // Maxlen 16 (with : + audio_prompt)
.UNUSED_2			= "",
.beep					= "Beep", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_1			= "Voix", // Maxlen 16 (with : + audio_prompt)
.transmitTalkerAlias			= "Tx TA", // Maxlen 16 (with : + .on or .off)
.squelch_VHF				= "Squelch VHF",// Maxlen 16 (with : + XX%)
.squelch_220				= "Squelch 220",// Maxlen 16 (with : + XX%)
.squelch_UHF				= "Squelch UHF", // Maxlen 16 (with : + XX%)
.display_background_colour 		= "Couleur" , // Maxlen 16 (with : + .colour_normal or .colour_invert)
.openGD77 				= "OpenGD77",// Do not translate
.openGD77S 				= "OpenGD77S",// Do not translate
.openDM1801 				= "OpenDM1801",// Do not translate
.openRD5R 				= "OpenRD5R",// Do not translate
.gitCommit				= "Commit Git",
.voice_prompt_level_2			= "Voix L2", // Maxlen 16 (with : + audio_prompt)
.voice_prompt_level_3			= "Voix L3", // Maxlen 16 (with : + audio_prompt)
.dmr_filter				= "Filtre DMR",// MaxLen: 12 (with ':' + settings: "TG" or "Ct" or "RxG")
.UNUSED_4				= "",
.dmr_ts_filter				= "Filtre TS", // MaxLen: 12 (with ':' + settings: .on or .off)
.dtmf_contact_list			= "Contacts DTMF FM", // Maxlen: 16
.channel_power				= "Pce Canal", //Displayed as "Ch Power:" + .from_master or "Ch Power:"+ power text e.g. "Power:500mW" . Max total length 16
.from_master				= "Ma�tre",// Displayed if per-channel power is not enabled  the .channel_power
.set_quickkey				= "D�fini Quickkey", // MaxLen: 16
.dual_watch				= "Double Veille", // MaxLen: 16
.info					= "Info", // MaxLen: 16 (with ':' + .off or.ts or .pwr or .both)
.pwr					= "Pwr",
.user_power				= "Pce Perso.", // MaxLen: 16 (with ':' + 0..4100)
.temperature				= "Temperature", // MaxLen: 16 (with ':' + .celcius or .fahrenheit)
.celcius				= "�C",
.seconds				= "secondes",
.radio_info				= "Infos radio",
.temperature_calibration		= "�tal. t�",
.pin_code				= "Code Pin",
.please_confirm				= "Confirmez", // MaxLen: 15
.vfo_freq_bind_mode			= "Freq. Li�es",
.overwrite_qm				= "�craser ?", //Maxlen: 14 chars
.eco_level				= "Niveau Eco",
.buttons				= "Boutons",
.leds					= "DELs",
.scan_dwell_time			= "Dur�e Scan",
.battery_calibration			= "�tal. Bat.",
.low					= "Bas",
.high					= "Haut",
.dmr_id					= "DMR ID",
.scan_on_boot				= "Scan On Boot",
.dtmf_entry				= "Entrez DTMF",
.name					= "Nom",
.UNUSED_3				= "",
.openDM1801A 				= "OpenDM1801A", // Do not translate
.time					= "Heure",
.uptime					= "En Funct. Depuis",
.hours					= "Heures",
.minutes				= "Minutes",
.satellite				= "Satellite",
.alarm_time				= "Heure Alarme",
.location				= "Emplacement",
.date					= "Date",
.timeZone				= "Fuseau",
.suspend				= "Veille",
.pass					= "Passe", // For satellite screen
.elevation				= "El",
.azimuth				= "Az",
.inHHMMSS				= "ds",
.predicting				= "Pr�diction",
.maximum				= "Max",
.satellite_short			= "Sat",
.local					= "Locale",
.UTC					= "UTC",
.symbols				= "NSEO", // symbols: N,S,E,W
.not_set				= "NON D�FINI",
.general_options			= "G�n�rales",
.radio_options				= "Radio"
};
/********************************************************************
 *
 * VERY IMPORTANT.
 * This file should not be saved with UTF-8 encoding
 * Use Notepad++ on Windows with ANSI encoding
 * or emacs on Linux with windows-1252-unix encoding
 *
 ********************************************************************/
#endif /* USER_INTERFACE_LANGUAGES_FRENCH_H_ */
