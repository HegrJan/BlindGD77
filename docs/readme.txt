AccessibleGD77 Readme.txt file by Joseph Stephen VK7JS.

Please note, if you are reading this file in a text editor, please ensure word wrap is enabled 

This file contains the change log of all changes made since Roger's rerelease of the official OpenGD77 firmware from which AccessibleGD77 was forked back in April 2021.
There are versions of firmware in this archive for the following radio models:
Radioddity GD77, GD77S, Baofeng DM1801, DM1801A and RD5R.
If you would like to read the quickstart guide for the AccessibleGD77 firmware, open the docs folder in this archive and locate the file relevant for your radio model:
"AccessibleGD77 Quick Start Guide.txt" for the Radioddity GD77,
"AccessibleGD77S Quick Start Guide.txt" for the Radioddity GD77S,
"AccessibleDM1801 Quick Start Guide.txt" for the Baofeng DM1801,
"AccessibleDM1801A Quick Start Guide.txt" for the Baofeng DM1801A (which has two less keys than the DM1801),
"AccessibleRD5R Quick Start Guide.txt" for the Baofeng RD5R.
The docs folder also contains help files produced by Ian Spencer, DJ0HF, which may assist you in getting started with the CPS software, the software used to communicate with your radio from your computer in order to upload firmware, voice prompts and your channel data (known as a CodePlug).

10 November 2021
1. Added new Voice Volume menu item to Sound Options menu. this allows you to adjust the voice volume from 10 to 100% in increments of 5% relative to the voice's standard volume. This is useful if your radio's voice volume is disproportionately louder than your signal reception volume. 
2. Added new Voice Rate option to the Sound Options menu. This allows you to choose from ten voice rates starting with the default rate of the installed voice pack file and increasing in rate from the voice's default in increments of 10%. Thanks to Bill Cox, AK3Q for his Sonic Lite library used for speed and volume adjustment. Given that you can now adjust voice rate on the fly, I am now only distributing two sets of voice prompts at the base speed of 1.25 and 1.75 times the original set.
3. Added a Voice menu to the GD77S to allow voice volume and speed changes on that model. Press Orange until you hear Voice, then use SK1 to increment volume or SK2 to increment speed. The values will wrap around after their maxima. This was more efficient than adding two different menu options.  
4. Voice prompts have been added for Voice volume and rate so please update the voice prompts on your radio from the included archive.
5. Fixed a bug in the reorder channels feature. If you held down left or right to swap current with first or last, it would first swap current with prior or next, and then swap current with first or last.
6. Fixed playing of splash screen boot melody when using generic power-on melody rather than custom tune.
7. Fixed Channel Summary (long hold SK1) at Voice Prompt Level 1 so it has reduced verbosity like Level 2. Previously it sounded like level 3 with all field titles being announced. Also, at level 3, always speak field titles, previously, if summary key was pressed while the radio was talking it would act like level 2 and not read all field titles.
8. Code cleanup: Now use same function to read channel number/name when reordering, for the summary feature, and for channel selection.
9. Reduced verbosity when selecting channels at vp level 2: No longer say fm or dmr, but, for dmr channel, announce tg after channel name. Thus if you only hear the channel name, its an analog channel, if you hear a tg, you know it is digital. If you want the more verbose mode announcement, choose vp level 3.
10. Reduced verbosity of vp level 1 when pressing sk1, it has taken this long for anyone to point it out. Pressing sk1 now matches vp level 2 rather than vp level 3.
11. Fixed a bug with Priority channel indication at vp level 3. Previously the wrong channel might be indicated as the priority channel, or, there'd be a discrepancy between what the channel summary indicated as the priority channel and what was indicated as the priority channel when selecting channels with up and down arrow.
12. Cleaned up and separated quick start guides for the main radio models.
13. GD77S specific changes:
13.1. The mode which used to be called Channel Mode where SK1 and SK2 changed squelch or talkgroup depending on the mode of the radio is now called Squelch Mode or Talkgroup Mode, depending on whether the radio is in FM or DMR mode.
13.2. The mode which used to be called Scan Mode is now called Channel Mode. From Channel Mode, SK starts or stops a scan, and SK2 now toggles between Channel and VFO mode. Note that pressing SK2 during a scan adds a channel or frequency to the nuisance list. Scan now works in VFO mode and scans from the current vfo frequency 1 MHz and cycles around to the start frequency again. The scan step is either 25 KHz or 12.5 KHz depending on the bandwidth set for the VFO.
13.3. The modes have been reordered slightly to make it easier to quickly get to keypad mode. The order is now Zone, Squelch/TG, Channel, Keypad, DTMF contacts, etc. Remember you can always cycle backward with SK1+orange.
14. Fixed a bug on the GD77S where issuing the command to clear the CTCSS/DCS tone in keypad mode, c, d, c0 or d0, set the code to a DCS code rather than clearing it. I was setting to 0 rather than the special constant which meant none. 
15. Created and updated separate accessible guides for each model of radio. See the docs folder for these guides.
Please note! If this is the first time you've updated your firmware during this development  cycle, your radio will perform a settings update (i.e. restore settings to their factory defaults). This will not change your CodePlug but you will have to reset your favorite options in the menus such as band limits, voice prompt level, etc.


5 October 2021
Please note! This update requires new voice prompts.
Also, all menu settings will be restored to factory defaults. You will need to set your various preferences in the Options and other various menus.
1. Standardized cursor placement when focus lands on, or when arrowing to, an edit field. The cursor is now always placed after the last character (unless the field is full, in which case it is placed on the last character).
2. Reordered Main Menu options to be more logical and to place more frequently used options near the top. I also put AutoZone last so you can up arrow once from Zone to get to AutoZone.
3. Fixed Radio Information screen so you can clear the Callsign and DMR ID fields completely in the event you wish to hand the radio to someone else.
Previously the radio would not let you clear the Callsign field, it had to have at least one character in it, and the DMR ID would show 0 rather than a blank field if empty which meant that you had to delete the 0 before entering the new ID rather than just entering the new ID. 
4. The GD77S and GD77 now use the same channel summary function. This means that the GD77S now anounces if the bandwidth is n for narrow after the mode for a channel using narrow FM. It also means that it doesn't read both rx and tx tones if they are the same, it simply says tone, and only specifies if it is rx or tx if either only one is set or if they differ.
It also means I could get rid of some mostly duplicate code.
5. Added US Rail AutoZone to the AutoZones available.
6. Added new Options mode to the GD77S and moved all global modes from the Orange function cycle to this new mode. In this new mode, similar to the Keypad mode, the channel knob is used to select the option and SK1 and SK2 are used to change the option's value. Long hold SK1 repeats the current option and its value, and long hold SK2 resets the option to its factory default.
This Mode contains the following options:
* Master Power,
* FM Mic Gain,
* DMR Mic Gain,
* FM Beep,
* DMR Beep,
* Band Limits, (off, on, CPS)
* VHF master Squelch,
* 220 MHz master Squelch,
* UHF master Squelch,
* TOT (Timeout timer) master (needed for PTT latch),
* PTT Latch,
* Eco Level (5 levels of battery save mode),
* Vox Threshold,
* Vox Tail,
* AutoZone,
* Firmware Info.
7. Added PTT Latch support to GD77S. Enable TOT (timeout timer) and PTT Latch from the Options menu. Note that PTT Latch will not work without a Timeout defined. This is by design. Also, when PTT Latch is enabled, like on the GD77, beeps are automatically enabled when you press and release PTT so you know when transmission has started and stopped.
8. Added Vox support to GD77S. Choose it using the Orange function key. It is after Keypad in the list. Turn on and off with SK1 and SK2. Set threshold and tail from the Options mode.
9. Reworked the Splash screen so that the boot melody is played and the custom text is spoken (at vp level 3 and higher). This allows you to hear the identity of your radio in the event you are meeting with other blind hams with the same radio. To set the two lines of custom text, either do it through the Information screen on the GD77, or via the CPS for the GD77S. You can leave both blank if you don't wish anything to be spoken, or just put text in line 1 if that is all you wish to do.
10. Added temperature announcement to long hold orange for GD77S since it doesn't have a radio info menu option.
11. Fixed switching from an autoZone which does not allow TX (such as US Rail), to an autoZone which does, and tx still being disabled.
12. Fixed an issue on the GD77S where you could be placed in the AllChannels zone which is not supposed to be supported on the GD77S. This could be seen if you were focused on an autoZone and then you disabled all autoZones.
13. Added ability to toggle between numeric and alphanumeric mode in edit fields using SK2+Right. This means that when you are editing a DTMF contact, and you start off in numeric mode, where the majority of your entry is numbers, but then you wish to add a letter, you can now press SK2+Right to switch to the keypad preview mode for entering alphanumerics. This works like the normal telephone alphanumeric keypad where multiple presses of a key cycles between letters and number and then a delay inserts that character into the field. You can then toggle back to numeric mode using the same key combination, SK2+Right. This works in DTMF fields and normal alphanumeric fields. It does not work in purely numeric fields by design.
Note that when a field gains focus, it always starts off in the most appropriate mode. E.g. the DTMF code field always starts off in DTMF mode where numbers, * and # insert their number or symbol. Name fields always start off in alphanumeric mode where multiple presses of the keys cycle between letters and numbers. Frequency and DMR ID fields are always in numeric mode, where only numbers are valid and you can't toggle the input mode.
14. Added new voice prompts. Please update your voice prompt file from one of the voices in this archive. Remember, the number after the prompt reflects the speaking rate of the voice relative to normal speed. 
14.1. The "information" main menu option is now spoken as User Information.
14.2. The name field in that screen is now "call sign" (to match the CPS software for the same field),
14.3. When toggling between alphanumeric and numeric with SK2+Right in edit fields, the radio will now say "alphanumeric" or "numeric". The input mode will also be indicated when using SK1 to repeat a field being edited.
14.4. Fixed the Mathew voices which were not being generated correctly due to a logic error in the python build script.
15. Fixed a bug on the Channel details screen where the name field would be errantly cleared and reported as empty when you hadn't edited it.
16. Added Delete from Zone and Delete from All Zones quick menu options to Channel Screen quick menu.
16.1. Delete from zone will delete the current channel from the current zone. Note that you'll get an error beep if you try and delete a channel from the All Channels zone as it is not a true zone. You also can't delete a channel from an AutoZone.
16.2. Delete from All Zones will delete the current channel from every zone on the radio , i.e. delete the channel completely from the codeplug. Again this will not work in an AutoZone.
17. Implemented ability to reorder channels in a zone. Choose "Reorder Channels" from the Quick Menu (orange button). When in reorder mode,  up and down select the channel to move, and left and right move that channel to the previous or next position in the zone, swapping it with the channel at the previous or next position. Press and hold left or right to move the current channel to the first or last position in the zone. Press red or green keys to exit the reorder mode and return left and right to normal channel screen operation.
Note: this does not work in the All channels zone or any AutoZone as these are not real zones.
18. Added Zone announcement to channel summary on long hold SK1.


9 September 2021
1. Added new Information screen to the Main Menu. This allows the user to set their callsign, their DMR ID, and two lines of text shown on the boot screen. Previously all except the user's DMR ID had to be set via the CPS software.
2. Implemented new, consistent edit handler. This edit handler will be used for all input fields where practical. This new handler will bring a consistent user experience wherever it is used. Previously, almost every input field used its own implementation which meant that there was no consistency. Some fields allowed editing, some did not. The edit handler supports the following:
a. Arrowing left and right in the field and speaking the character moved to,
b. SK2+Left backspace and speak backspaced character,
c. Long hold Left, home and speak first character in field,
d. Long hold Left+SK2 delete to start of field.
e. Long hold Right, end (moves to last character and speaks it if it is not blank),
f. Long hold Right+SK2 delete to end of field. 
g. Proper character insertion until the field is full at which point characters will be overwritten,
h. Proper alphanumeric preview and speaking for alphanumeric fields,
i. Simple digit input for numeric fields,
j. Simple DTMF support for DTMF fields,
k. SK1 will repeat the content of the edit field. 
The following fields have been updated to use the new edit handler:
* Channel Details Screen: name, rx freq, tx freq, DMR ID,
* DTMF contact details screen: name, code,
* DMR contact details screen: name, TG/PC,
* DTMF entry, TG entry, PC entry, user DMR ID entry screens,
* Radio Details screen: name (callsign), DMR ID, boot line 1 and 2.  
* The following has not been updated: direct channel/frequency entry, scan ranges. These will probably not get updated, though I may rework scan ranges at some point. 
3. Fixed bandwidth toggle all radios. the * key on the models with keypad, and the b command on the GD77S would announce the bandwidth change but the radio did not actually change bandwidth.
4. Implemented a requested shortcut to return to Channel Mode from any mode on the GD77S. If you rotate the channel knob from any mode (other than Keypad mode), the GD77S will immediately return to Channel Mode. For example, if you hit the orange key to change to say Power Mode, a quick flick of the rotary knob up and back will return the radio to Channel Mode without having to press the Orange key multiple times to get back to Channel Mode.
5. Implemented another requested feature, Copy Channel to VFO, on the GD77S. If you just want to copy a channel to the VFO without issuing any command to change the current channel's settings, after locating the channel you wish to copy, activate Keypad mode, then immediately press long hold orange with an empty keypad buffer. The current channel will be copied to the VFO. If you previously visited keypad mode, remember to clear the keypad buffer with long hold sk2 prior to pressing long hold orange. In other words, previously you had to issue a command such as changing frequency, bandwidth, CTCSS/DCS code, etc, before long hold orange, to save the settings with the command applied. This enhancement merely lets you copy  settings to VFO without changing them first.
6. Removed artificial forcing of master power on GD77S so you can set custom power levels via the CPS.
7. When the received TG is different to the transmit TG, the indicator beep is now played at voice prompt level 1 and higher, not level 2 and higher.
8. Normally, the Power mode on the GD77S will adjust the channel or master power but there is no way of either clearing a channel's custom power or setting it if it is from the master. Thus, I've added a new Keypad mode command ** number, followed by long hold Orange, to do that.
8.1. **0 will clear the channel's custom power, forcing it back to the master value.
8.2. **1 through **10 will set the power to levels 1 through 10, where 1 is 50MW, 9 is 5W etc.
8.3. **50, **250, **500, **750, **1000, **2000, **3000, **4000, **5000, or **5100 etc will set the channel's custom power level to the nearest milliwat level supported by the radio.
8.4. The channel summary on long hold sk1 will now announce the channel's power level. If it is from the master value, the "from master" announcement will be spoken after the power level.
9. RSSI screen: Added continuous tone feedback while SK1 is held down.
            Works for DMR only. (Thanks to Jan, OK1TE).
10. Added Mic Gain to GD77S Orange function menu. This will either adjust FM or DMR mic gain depending on the current channel.
11. There is an acknowledgement beep when you reach the edit where you enter the user DMR ID from SK2+# when in DMR Contact mode. The probloem is, if voice prompts are active, it doesn't get played until it is irrelevant, i.e. you've begun editing and arrowed beyond the edit boundary such that there is no character to be announced. The ascending tones now only get played if either the audio prompt level is set to beep, or if the level is higher than beep and no other voice prompt is playing. This way the melody gets played when it is relevant, or not at all, rather than at a later time when it serves only to confuse the user.
12. SK1+orange will now cycle backward through functions on the GD77S. This is not perfect because we have to detect the intention of the user and then suppress secondary functions of SK1 until both keys are released, but it is as good as we can get it. SK1 was chosen as its secondary functions are less invasive than those of SK2 in the event the secondary function is accidentally activated.
13. Fixed SK2 when scanning on the GD77S so it properly adds the current channel to the nuisance list during a scan.
14. Added TG to channel announcement on GD77S for DMR channels.

19 August 2021
1. The GD77S now has a virtual keypad mode. Enter it like other modes, by pressing Orange until you hear Keypad.
1.1. Once in keypad mode, the rotary knob is no longer used for channel selection but number and letter selection.
1.2. Read the current buffer with long hold sk1.
1.3. Add the currently selected digit or letter to the buffer with short press sk1.
1.4. Backspace the last char with short press sk2.
1.5. Clear the buffer with long hold sk2. You'll hear a confirmation beep.
1.6. If you hit ptt, the buffer is sent as dtmf tones (if the radio is in analog mode).
1.7. If you press long hold of orange, the buffer is interpreted as either a command (see below) or a frequency, or frequency pair (rx tx) and the radio is set to that frequency.
1.7.1. The frequency entry is flexible. If it is simplex, you can enter as few digits as you need to get the correct frequency, e.g. 147 for 147 mHz.
1.7.2. If you wish to enter both rx and tx, so long as the rx starts at position 1 and the tx at position 9, i.e. first 8 for rx and 2nd 8 for tx, it should work. The following will work:
14700000146400 
To set rx to 147 and tx to 146400.
Or,
1470000014640000 i.e. rest filled with 0s.
Thus, if you wish to set frequency then dial a dtmf string, first set the frequency, then long hold sk2 to clear and then enter the dtmf code, then ptt.
Note, even if you exit the mode with short ppress orange, then go back in, the buffer is preserved.
2. You can now enter a special command string followed by long hold orange to change radio functions. Commands which are supported by the virtual keypad include:
2 .1. * followed by long hold orange will toggle radio mode. Note, since the buffer is not automatically reset, pressing long hold orange again will cycle the mode back again.
2.2. # folowed by long hold orange will toggle time slot in DMR mode.
2.3. # followed by digits followed by long hold orange will set the radio to the specified talkgroup if in DMR mode.
2.4. ## followed by digits followed by long hold orange will start a private call with the user with the specified DMR ID.
2.5. ### followed by ID followed by long hold orange will set the radio's User DMR ID. Note this is saved to flash memory so power cycling the radio will maintain this user ID.
2.6. B followed by long hold orange will toggle radio bandwidth in FM mode. Note, since the buffer is not automatically  reset, pressing long hold orange again will cycle the bandwidth back again.
2.7. C followed by digits followed by long hold orange will set a CTCSS code, e.g. C854 would enter the code 85.4 hz tx tone. Add a * to set the code for rx also, e.g. C854* would set rx and tx tone to 85.4 hz.
2.8. D followed by digits followed by long hold orange to enter a DCS code, e.g. D31 would enter the DCS code 031  tx DCS code. Likewise adding a * after the digits will set rx dcs also.
Note to clear a CTCSS or DCS code, enter c followed by long hold orange.
Remember, if you wish to enter a new command, don't forget to clear the buffer first with long hold sk2.
2.9. Command A or  A1 through A16 followed by long hold orange, recall VFO, copy VFO to channels 1 through 16 in the current zone.
Whenever you go to keypad mode, whatever channel was last active is still active. If you then change the radio mode, enter a frequency, tone, set mode etc, whenever you press long hold orange to action a command, all of the current VFO settings are automatically saved. You can then go and do other things, change channels etc, even turn off the radio.
When you go back to keypad mode, as usual, whatever was last active is still active, however, you can recall the prior saved VFO settings using the command a followed by long hold orange. This makes the saved VFO settings active again.
If you wish to make the VFO settings more permanent, you can then save these settings to channel 1 through 16 in the current zone using the command a1 through a16 followed by long hold orange. All of the VFO settings are saved to the specified channel in the current zone. The overwritten channel will have its number as its name. e.g. channel 1, channel 16, etc. Of course you cannot save to an AutoZone channel.
3. Added beep and flush of speech when clearing buffer on long hold sk2 or when buffer is emptied with backspace with sk2.
4. Autozones support for the GD77S has been added
4.1. Cycle to the AutoZones mode with the Orange key.
4.2. Use SK1 to cycle through the names of the AutoZones and use SK2 to enable or disable an AutoZone.
4.3. Once you've enabled or disabled all the autozones that you want, you can then choose them from the Zones mode.
4.4. If an AutoZone has more than 16 channels, use long hold SK2 to select the next channel bank. Each time you long hold SK2, you'll move up by 16 channels. (This is because the GD77S has 16 physical channel positions.). Once you've reached the last bank, long hold SK2 will return you to bank 0.
4.5. If an AutoZone has duplex available for a particular channel, such as a UHF CB channel, an extra long hold of SK1 will toggle between simplex and  duplex mode. (You'll hear none for simplex and either plus or minus for a plus or minus repeater offset as defined by the AutoZone). 


7 August 2021
Please note that after this firmware update, your settings will be reset to factory defaults and you will need to set them again. This is necessary to avoid issues with memory corruption due to the settings being expanded for some of these new features.
1. Fixed PTT Latch so the confirmation start/stop beep is not heard by the other station, also changed beep to the shorter DMR start/stop beep.
2. Added FM Beep option to the Sound Options menu. This is like the DMR Beep option, i.e. you can control if you want to hear a beep when you press PTT in FM mode either at the start of tx, end of tx, or both. It is controlled independently of the DMR beep as some may wish to have it on for DMR but not FM. We still force it on when PTT Latch is in use however for safety.
3. Removed the AutoZone option from the Options Menu and added a new AutoZones menu to the Main Menu. This allows you to enable multiple Auto Zones. Each AutoZone can be turned on or off from this menu. If an AutoZone is turned on, it is available when cycling through the zones with sk2+up/down or from the Zones menu.
4. Fixed MURS bandwidth, I had channels 1-3 and 4-5 around the wrong way. I.e. Channels 1-3 need to be narrow and 4-5 can be wide.
5. Fixed default GMRS bandwidths and power levels.
6. Added FRS Autozone with correct default bandwidths and power levels, restricted to simplex and restricted channels.
7. Reworked AutoZones to be more memory efficient by using a lookup table for initial data, rather than helper functions.
8. When a tone scan is performed in VFO mode, and a tone is detected, it is now announced. It can then be repeated with sk1. The channel details (sk2+green) were already automatically updated with the detected tone, its just that the tone was not anounced for the blind user and thus then had to be interrogated.
9. Fixed issue: If you attempt to copy VFO to a new channel and the channel screen is in an AutoZone, we now copy the VFO to the All Channels zone as you can't copy to an AutoZone.
10. We now remember the zone channel when switching zones so that when you switch back to a zone, you are on the last channel you were on prior to switching away from that zone. (It used to really annoy me having to reset it each time). Note this will only work for the first 32 zones, but it is unlikely that most people would have even that many. (You can apparently have up to 250 zones.)
11. Fixed exiting from AutoZones menu to return to main screen like Options and other top level menus if Green key is pressed.
12. Other code cleanup.
13. Fixed power announcement when pressing SK2+Left in VFO mode. This was a casualty of the large merge a few weeks back.

28 July 2021:
1. PTT Latch enhancements:
1.1. Added Master TOT (Timeout Timer) setting so it does not have to be set on each individual channel. this is necessary because the PTT Latch feature requires a TOT.
1.2. The channel details screen now shows "From Master" if TOT is not set.
1.3. The PTT Latch feature now emits a beep when PTT is toggled on or off. Previously a blind user had no indication and could leave PTT latched unintentionally. I used the same beeps as used by the SK2Latch feature.
2. Fix to stop hearing part of "Keypad locked" message when you press SK2 when SK2 Latch is enabled, or hearing the full message  when the SK2 latch is disabled, when attempting to unlock the keypad. When SK2 latch is enabled, we want to hear the double beep, not part of the message. When SK2 latch is disabled, we do not want to hear the message while holding SK2 when pressing * to unlock. (Note this used to work prior to the merge but the new firmware timing broke it).
3. Added International Marine AutoZone to AutoZones options menu. This corresponds to the international Marine band as outlined at https://www.acma.gov.au/vhf-maritime-mobile-band-channel-allocations
 4. Fixed GMRS AutoZone to contain complete GMRS channel allocation with correct naming and order.
5. Added MURS AutoZone.
6. Added PMR446 16 channel autozone.
7. Fixed long hold red key in AutoZones, even though the radio said it set the priority channel correctly, the channel was out by 1.

23 July 2021
1.     Fixed various issues with AU UHF CB autoZone.
1.1. If you went to an RX only channel then visited a channel on which tx was permitted (eg 22 to 21), you could not tx even though it should be permitted.
1.2. If you tried changing from duplex to simplex on a repeater channel, you'd still be using the repeater offset even though the voice said it was off.
1.3. If you changed from a duplex to a simplex only channel, one could still cycle repeater offset even though it should not be allowed.
1.4. Fixed red key long hold to switch between priority (ch5) and current channel for this autozone.
1.5. Fixed Autozone Options Menu so that when arrowing right, we stop at the last value and do not wrap.
1.6. Fixed issue of when cycling autozone to off and having the zone reset. Previously because we made the Autozone active as soon as you chose an autozone, when we cycled back to off, we'd forceably reset the zone. We no longer make the Autozone active so it doesn't get reset.
2. Merged all latest changes from official OpenGD77 firmware. The relevant changes from the forrum are listed below:
* Fix display bug:
- if VFO and Channel are set on the same frequency, but different mode, the screen is displaying the DMR QSO info on FM screen.
- some delayed rendering was done on the wrong screen.
* Fix voice prompts:
- crashing.
- silent prompts.
- less cracky sound (w or w/o receiving signal)
- wrong menu name announcement.
* Enhance Monitor mode with DMR detection first.
* GD-77S:
- clear TS override when a channel is selected (otherwise it's stuck on this overridden TS).
- fix "select tx" VP not announced on SK2 longpress.
- fix heart beat in RED only.
- fix TX stuck bug.
- fix heart beat leds blinking (Eco level > 0, no more fake event)
- add Eco Level menu mode (use default as 1, like every platform).
- leave to the hotspot code the full control of the LEDs: no heartbeat.
* Toggles VFOs on LongPress <RED> (unsupported on the DM-1801).
* Fix manual mode backlight.
* Fix low battery handling (false triggering).
* Battery level in the header line is now blinking on low battery.
* Add DM-1801A support (no Left/Right keys, VFO/MR and A/B used instead).
* Small cleanup with CTCSS.
* Lot of internal changes/fixes.
* VFO sweep/ spectrum
VFO screen
Long press #
Left / Right : Step down or up central frequency of sweep
SK2 + Left / Right: Change overall sweep bandwidth aka zoom
Down / Up: Decrease / increase virtual gain
SK2 + Down / Up: Decrease / Increase virtual noise floor
SK1 + Up or Down: Resets the gains/floor to default
Monitor mode (long press on SK2) suspends the scan and open the receiver to the central frequency.
* long hold of red from a submenu will escape back to channel or VFO screen.
* In the Channel Details screen, you can now enter a DMR ID for a DMR channel, rather than it being read-only.
3. Added DTMF Entry to the Quick menu available from the DTMF Contact List.
This gives access to the DTMF dialer in the official firmware. From this dialer, left=a, up=b, down=c, right=d, Green=dial, Red=cancel. If the DTMF contact list is empty, the quick menu will now contain both DTMF Entry and New as options.
4. Fixed the Channel Screen quick menu filter announcement broken in the initial beta.
5. You will need to upload new voice prompts for this release, even if you did it for the initial beta, the files had to be updated again. They should now be compatible with the official OpenGD77 firmware. All the voice prompts are now in the same folder with their speed appended to the name of the voice prompt file, similar to the official Open GD77 release. We have four versions of each prompt file, one at 1.0 speed, one at 1.25 x speed, one at 1.5 x speed and one at 1.75 x speed. 
To confirm you have the correct voice prompt file, you can do two tests:
a. Go to Menu, then Contacts, then DTMF Contacts List. Then bring up the Quick Menu (Green key) and arrow up once to DTMF Entry, which is the new dialer option. Ensure the radio says "DTMF Entry".
b. From the same Quick Menu, arrow down to Edit Contact and hit Green. Arrow down from the Name field to the Code field and the radio should announce "Code".
If both of these tests pass, you have the correct vpr file loaded.
 
12 July 2021
    1. Fixed Voice Prompt level 1 low and high beeps when toggling the following:
    1.1. Toggling of bandwidth in VFO mode.
    1.2. Toggling between VFO A and B (long hold red).
    1.3. Toggling between current channel and priority channel (long hold red).
2. Added new AutoZones feature.
An AutoZone is a pseudo zone created on the fly based on channelized frequencies calculated via a formula with heuristics applied. I.e. these zones do not take up any memory for the channel data.
2.1. A new option has been added to the Options Menu called Auto Zone.
2.2. The settings for this option are off (no AutoZone), AU UHF CB (The Australian 80 channel UHF cb band), NOAA, and GMRS.
2.3. When you enable an AutoZone, the AutoZone is immediately made the active zone but SK2+up/down will cycle you to the other zones.
2.4. When disabled, the AutoZone will disappear from the zones through which you may cycle.
2.5. When an AutoZone is active, the rules for that band are adhered to, for example:
2.5.1. In the Australian UHF cb band, channels 22, 23, 61, 62 and 63 are receive only, and duplex (hash key) may only be enabled on channels 1 through 8 and 41 through 48. The correct repeater offset (750 kHz) is automatically applied when duplex is toggled.
2.5.2. When NOAA is active, all channels are receive only.
2.5.3. When GMRS is active, duplex (hash key) may be used on channels 1 through 8 and the correct repeater offset is applied. Channels 9 through 15 (the interstitial channels) only allow simplex operation.
2.6. While you can change things like CTCSS, the values cannot be saved permanently since the channels don't really exist. Thus, you may only change these settings temporarily via the Channel Details screen.
2.7. While the UHF CB band defines a priority channel, I have not had time to hook this up to long hold red yet. I shall fix this when I get time.
 

7 July 2021
1. Removed "From Master" message when announcing power level when summary key is pressed in VFO mode as it is always from master in VFO mode.
2. Added Repeater Offset feature:
2.1. Added VHF Offset and UHF Offset menu items to Options menu.
2.2. The defaults are 600 kHz for VHF and 5000 Khz for UHF. The VHF offset can be set anywhere from 100 to 1000 kHz in steps of 100 kHz. The UHF offset can be set from 100 kHz up to 9.9 mHz in steps of 100 kHz.
2.3. In Channel or VFO analog mode, # will cycle between off, plus and minus.
2.4. The idea is that you enter a frequency in VFO mode and then press the hash key to quickly set the repeater or simplex offset appropriate for the current band, then go to quick menu to write the channel to a new blank channel.
2.5. Note that the repeater offset is always set to none when you change channels, Enter VFO mode or when you enter a new frequency. This is to avoid confusion and conflict with the other features such as Bind RX and TX frequency, and so you always know to press hash once for plus offset, twice for minus offset, and thrice if you wish to ensure the frequency is set to simplex (to reset any binding of rx and tx frequency which may have been a result of a prior set).
2.6. Pressing hash is simply a quick way to ensure you are either set to a simplex frequency (offset set to none), or to the correct repeater offset.
3. Changed announcement of CTCSS (5 syllables) to tone. If there is a different tone for receive and transmit, the slow syllables r x and t x have been changed to receive and transmit which are spoken faster and with less pauses.
4. Added new voice prompts for the new features, so please ensure you update your voice prompts.
5. Corrected a display issue on the DTMF edit screen introduced when I merged Roger's changes in.
6. Fixed an issue when setting the Priority Channel from the Options menu. If the code plug had unused channels, e.g. 1, 2, 3, 5, 10, when arrowing left and right you'd hear blank places where the unused channels were.

5 July 2021:
1. If you dial a DTMF contact from the contact list, we now inhibit the initial menu voice prompt so you can hear the results of dialing the contact, i.e. the resultant status message. Previously the menu announcement would be played over the status mesage.
2. After consultation with users, I removed the n for narrow indicator from AnnounceRadioMode so that arrowing up and down through channels will not indicate this. I added it instead to after the call to AnnounceRadioMode in the summary function so that it is indicated after the mode only when the summary key is pressed.
3. Added Summary (Long hold SK1) to VFO mode.
4. Added new voice prompt folder with faster voice prompt files. These are 10% faster than the original. I could not make them any faster as they became largely unintelligible. See the  folder called "voice prompt files (10% faster)". I also removed voicem.vpr and voicef.vpr from the root of the archive as they are duplicates of Brian and Amy in the Voice Prompts folder.
5. Increased the maximum SK2 latch time to 10 seconds.

27 June 2021
1. The Caller/Callee submenu from the Last Heard list has been changed so it now shows the ID of the individual, and the Talkgroup that individual was talking on. This is more explicit than Caller and Callee. If you choose the ID of the individual with the Green key, you will start a private call with that individual. If you choose the talk group, you will be transmitting on that talk group next time you press PTT, similar to when receiving a different talk group to the one you are currently transmitting on and you hit the SK2 button.
2. Fixed announcement of Quick Menu on both the DTMF/DMR contact list Quick Menu and the Last Heard list Quick Menu. Previously when you hit Green, you did not know that you were on a submenu. (OpenGD77 calls a submenu a Quick Menu).
3. The Radio Mode announcement has been modified to distinguish narrow fm from wide fm. The radio will now say nfm for narrow and just fm for wide. 
4. I changed the Summary (long hold SK1) to speak nfm or fm (in analog mode) rather than the long and verbose string "bandwidth 25/12.5 kHz", as the summary was getting rather long. The kHz announcement is still spoken on the * toggle however.

22 June 2021
1. I have renamed this version of the firmware to AccessibleGD77 to avoid confusion with Roger's version. This is in no wise to detract from his majority contribution to the firmware, but to distinguish this version from his. This rename applies to the splash screen, the version reported on the firmware screen, and the names of the binary files. You will need to update your voice prompt file.
2. You can now set the priority channel to a channel in a different zone to the current zone. If however the priority channel does not exist in the current zone, and it is currently selected, the summary key, long hold SK1 key, will not read the channel number since it is not relevant to the current zone. Thanks so much to Vítor - CT1AFS for his help testing this, it was a very difficult feature to implement, or as they say in English, a very, very hard nut to crack. 
3. Added Priority Channel to the bottom of the Channel Details screen (SK2+Green from Channel Screen, or choose from Main Menu). This shows if the currently displayed channel is the priority channel (by showing Yes). If it is and you hit left arrow, it is changed to no and the priority channel is unset. If it is no and you press right arrow, the currently displayed channel is set as the priority channel. Use Green to save temporarily or SK2+Green to save permanently, like all Channel Details screen changes. This is an alternate way to set a priority channel rather than going to the Options menu.
4. Added new submenu to Last Heard screen, thanks to OK1TE. If you hit the green key from the Last Heard list, you will now be presented with a submenu with two items, Caller and Callee. If you choose Caller, you will invoke a private call with the selected last heard station. If you choose Callee, it will be as if you hit SK2 in reception mode, i.e. your TX will be set to the Talk Group of the last heard station.
5. Added * in FM mode to toggle between narrow and wide FM modes in both channel and VFO screens (equivalent  to Roger's latest release). 
6. Added long hold of * in analog mode to reset the channel's bandwidth to its default (saved) state, similar to how long hold of * in digital mode will reset the time slot to the channel's default (saved) time slot.
7. Added bandwidth to channel summary (long hold SK1) in FM mode.
8. When SK2 Latch is enabled, and you press SK2 to latch it on the lock screen, you  now hear the latch double beep rather than the message "keypad locked". This makes it clearer that you can press the * to unlock the keypad.
9. Added pauses in the Firmware Info voice prompt to make it more comprehensible, particularly to separate the date and time.
10. Fixed SK2 Latch and DTMF Latch so that if you reset to factory settings, these options have sensible values. SK2 Latch is set to off and DTMF Latch is set to 1.5 seconds.

15 June 2021
1. Added Priority Channel and Priority Scan feature.
1.1. Set the Priority Channel from the Options menu. this is relative to the current zone. Use left and right to choose either none, or a channel to set as the priority channel.
1.2. Long hold of Red on the Channel Screen will toggle you between the priority channel and the last channel you selected. (This is similar in concept to what happens in VFO mode where long hold of Red toggles between VFO A and VFO B.)
1.3. Priority Scan has been added to the Channel quick Menu. This effectively starts a dual watch using the Priority Channel set in the Options menu, and whatever the current channel is (which can be changed on the fly by the user.)
1.4. the Summary key, long hold SK1, will indicate if the dual watch is a Priority Channel Scan or a regular Dual Watch, based on the initial channel.
1.5. The Scan On Boot feature has been extended. It now has three values: Off, Scan and Priority Channel Scan. The third option will do a priority channel scan rather than a regular channel scan on boot.
2. This feature requires new voice prompts so please update your radio with a voice prompt from this archive.
3. Updated txt files in the docs folder with Ian's latest version of the files.
4. Fixed 1750 tone when SK2 latch is enabled. If you press PTT, then SK2 to generate a tone, it no longer latches. Thanks OK1TE for this fix.
5. When unlocking with SK2+*, release SK2 Latch so you don't hear the double beep after the unlock.  

14 June 2021
1. I have slightly changed the folder structure in this archive:
1.1. All help files are now in a docs subfolder.
1.2. Added "voice prompt samples" folder containing mp3 samples so you can hear the voice quality prior to uploading the voice prompt file to your radio.
1.3. Renamed "Other voices" folder to "voice prompt files"
1.4. Added AccessibleGuideGD77.txt to docs folder. I also consolidated a couple of the other text files for simplicity.
2. Updated CPS software with latest from official OpenGD77 project.
3. Added long hold Red in VFO mode to toggle between VFO A and VFO B on GD77 (This is also still on double press of orange but that key is les intuitive and is prone to breakage on many radios).
4. Fixed issue of hearing the wrong channel when pressing sk1 when scan is paused due to receiving a signal.
5. Changed SK2 Latch so you can configure the timeout. If it was previously set to 1 for on, it will automatically be set to 3 seconds. The valid values are off, or 1 to 5 seconds in increments of half a second.
6. Added DTMF Latch option to Options Menu to disable or set the DTMF latch timeout. Valid values are off, and 1 through 3 seconds in increments of half a second.
7. this release requires an updated voice prompt file, so please ensure you upload a voice prompt file from this archive, or the radio will not announce the new DTMF Latch option in the Options menu.

9 June 2021
1. Fixed SK2 Latch feature so that if you press SK2 during DMR reception to change the TG to the incoming TG, the latch is automatically cancelled.

7 June 2021
1. Added Monitor Mode to RSSI screen (I.e. you can hold down sk2 to open squelch to hear the signal whose value is being monitored.) 
2. Fixed follow-on at voice prompt level 3 and reduced verbosity at voice prompt level 2 for the words contact/tg/private call.
3. Corrected link in instructions below.
4. Fixed RSSI screen which was broken in  Roger's version.

6 June 2021 VK7JS
All changes from the  2 June release have been merged with the official OpenGD77 release on 1 June.
This requires new CPS software to load the new firmware as well as new voice prompts.
There is a new procedure to update your radio which involves obtaining the official Radioddity firmware which is then merged with this firmware by the CPS software.
Warning, Warning, Warning, the old CPS software, sgl files and voice prompts cannot be used with the new procedure. 
This change was made necessary by the recent licensing issues which resulted in a major rewrite of the firmware and procedure to update it.

Instructions:
1. Download the official Radioddity firmware at the following link:
http://radioddity.s3.amazonaws.com/2021-01-26_GD-77_CPS_%26_Firmware_Changelog_Ham_Version.zip
2. Extract the archive and find the file called GD-77_V4.3.6.sgl which is the 4.3.6 version of the official Radioddity firmware.
3. Copy it into the same folder where you will upload the bin file in this archive from.
4. Power off your radio.
5. Connect it to your computer using the correct cable.
6. Hold down SK1 and SK2 (the two buttons below PTT), at the same time, while powering on the radio, to go into firmware loader mode.
7. Run the CPS software in this archive.
8. From the Extras menu, choose Firmware loader.
9. Choose the model of your radio 
10. The first time you do this, choose the Select Firmware button and point the dialog to the official Radioddity file: GD-77_V4.3.6.sgl
Once you've done this once, the CPS software will remember this location so you won't need to do it again.
11. Next, click on Select A file and Update button. (Each time you update the firmware after this point, you'll only need to do this part).
12. Choose the version of the firmware for your specific radio:
OpenGD77.bin for the Radioddity GD77,
OpenGD77S.bin for the Radioddity GD77S,
OpenDM1801.bin for the Baofeng DM1801,
or
OpenRD5R.bin for the Baofeng RD5R.
13. Once you hit Enter on the correct .bin file, the firmware will be uploaded to the radio. You'll hear the progress as the operation is performed.
14. Once the completion dialog appears, press ok.
15. Turn off your radio and turn it back on without holding any buttons down, to go to codeplug/voiceprompt upload mode.
16. Close the Firmware loader dialog if you haven't already done so.
17. Go back to the Extras Menu and choose OpenGD77 Support.
18. Tab until you get to the Write Voice Prompts button.
19. Choose the voice prompt file from this archive, e.g. voicem.vpr, voicef.vpr or one of the voices in the "other voices" folder.
20. Once the voice prompt file has been uploaded, you can turn off your radio, disconnect it and then turn it back on and use it normally.

This update (5 May 2021) contains all features and fixes from the past few months as outlined below in the change log plus Roger's latest features.


VK7JS Change Log:
5 May 2021:
1. Merged all features into new OpenGD77 codebase.
2. Generated new voice prompts. Note these are not the same as the ones distributed by Ian or Roger at this time. They may soon be merged however, as our voice prompt files contain extra words and corrected pronunciations for several words. The voice prompts may be merged in the future but I'll let you know if and when they are interchangeable.


2 May 2021 VK7JS
1. Moved SK2 Latch menu option to immediately after PTT Latch in the menu to group the two like options together.
2. Disabled SK2 latch for GD77s where it was possible to be on even though it was not relevant, depending on what was in memory.
3. When pressing SK2 to latch, silence any voice prompt that is in progress.

31 May 2021
Please back up your old sgl file in the event you have problems.
1. Added SK2 Latch feature. this is enabled via the Options Menu.
1.1. When on, you can press and release the SK2 button then press another button to activate the secondary function. for example, press and release SK2, then hit orange, to get battery level, or, SK2, then release then a quick key. You can press SK2, then release and hit * to unlock if the keypad is locked. You can also hold the secondary key down, e.g. press and release SK2, then hold down right arrow to increase power by multiple steps.
1.2. If you do not hit a second key within 2 seconds, the latch is released. You'll hear a double high beep when the latch is engaged, and a second lower double beep if it times out and the latch is released. You won't hear the beeps if the function generates its own voice prompt, such as when the keypad is locked.
1.3. You can cancel the latch immediately by hitting SK2 again.
2. Plese note that this new feature requires updated voice prompts so please update your radio's voice prompt using the voice prompt from this archive. If you do not do this, you will not hear the voice prompt for the new menu option in the Options menu.
3. Fixed SK2+up/down so that it reads the zone name with the same fix as in the prior update see 7 below. I now also announce the zone name at voice prompt level 2 and above rather than level 3 because the zone cycles and you would otherwise not know which zone you are switching to.

30 May 2021 VK7Js.
1. Added DTMF latch (similar to the Kenwood TH-D74A). This allows the user to press PTT, begin dialing DTMF codes, release PTT and continue dialing without having to hold PTT down. The radio will keep transmitting until a short timeout after the last code is dialled. This is ideal for people with RSI or other dexterity issues.
2. Added voice prompts to Credits screen. When you invoke the Credits screen from the main menu, you can now arrow through all of the credits and hear the main contributors to this project. Because of the limited memory space, and that voice prompts are recorded rather than synthesized on the fly, names are spelled, like on the Language and Zone menus. This completes the accessibility of this radio, and makes it the most accessible in the world with every single menu item now spoken and every function accessible by a blind user.
3. Brought Quick Key support on the channel and VFO screens up to scratch. My first implementation just immediately saved the quick key when you hit sk2+number, now, the same functionality has been implemented as on other main menus, i.e. when you press sk2+number, you hear "Set Quick Key x to y", where x is the key and y is the function. You can then hit the Green button to accept or the Red button to cancel the action.
4. Because english_uk_brian.vpr is included as voicem.vpr, and english_uk_amy.vpr is included as voicef.vpr in the root of this archive, they have been removed  from the "Other Voices" folder to save space in the archive.
5. Fixed an issue with DTMF latch breaking PTT latch. If PTT Latch was enabled without a TOT value defined in the channel 
details screen, PTT Latch was still engaging when it should not. I.e. The regular PTT Latch should only engage if enabled and a TOT (time out time) value is defined in the channel details screen.
6. Fixed voice prompt squelch tail on Baofeng DM1801 and RD5R radios. This was noticeable when using any function which provided a voice prompt on analog channels or frequencies.
7. Fixed the Zone Menu so that if the name of a zone begins with the word "zone", it will be pronounced rather than spelled. So long as the default zone names are used, e.g. zone 1, zone 2, etc, this will result in the zone names being spoken correctly. If the user changes the default names, the word zone will be pronounced and the following text spelled.

25 May 2021 VK7Js.
1.	Unified voice prompt announcements in all menus. This makes the announcements consistent for all Voice Prompt levels and brings the announcements to all the menu items. (Jan OK1TE).
2.	Dual Watch has been implemented for the Channel Screen. Here's how it works.
2.1.	Go to the channel you wish to monitor.
2.2.	Press orange button.
2.3.	Arrow up once to Dual Watch.
2.4.	Press the Green Button to selection the option. You are returned to the channel screen.
2.5.	Arrow to another channel, or use direct channel entry to select one.
2.6.	Dual Watch will begin after a couple of seconds or so (to give you time to arrow around without the channel changing on you.
2.7.	If you press and hold sk1, you’ll hear the two channel numbers you are monitoring.
2.8.	You can arrow to another channel etc and still monitor the original channel. When you arrow around to different channels, the Dual Watch will pause temporarily while you arrow and then begin again.
2.9.	If you hit PTT to transmit, Dual Watch is temporarily suspended on the current channel. When you release PTT, the Dual Watch will restart in several seconds according to the Scan Delay option in the Options menu.
2.10.	If you press the Red button, Dual Watch is cancelled with a confirmation message of the channel you are on but you will not go to VFO mode. You only go to VFO mode if Dual Watch is not active when you hit the red button. If you press the Green button, Dual Watch is cancelled and you go to the menu system. If either red or green buttons are used to stop Dual Watch, it must be restarted using the Orange button quick menu. Note that if you cancel Dual Watch using the Red or Green buttons, you are left on the channel you last arrowed to or used direct entry to go to, rather than in a random state.
2.11.	If you press and hold Up Arrow to begin a regular scan, Dual Watch is also cancelled and must be restarted manually.
2.12.	Previously, if the radio  was scanning and you hit PTT, the scan would stop but the radio would not transmit until you released and pressed PTT again. This has been fixed so that when scanning or in Dual Watch mode, PTT will stop the scan or suspend the Dual Watch, and immediately transmit.
3.	Fixed Quick Keys so they work on the Channel Mode or VFO mode Quick Menus. This allows you to assign a quick key to Dual Watch or any other Quick Menu function in either the Channel or VFO screens.

18 May 2021 VK7JS
Thanks to Ian Spencer, Danny Hampton, and Jamming Jerry  for help with testing.
1. Added a new DTMF Contact screen for creating new or editing existing DTMF contacts. Note only 32 entries can be added to this database. This is a limitation of the current CPS and memory allocation. Note that in this screen, character entry is in overwrite rather than insertion mode, i.e. inserting a character in the middle of a field replaces the character at that location and moves the cursor one character to the right. 
1.1. Moved New Contact option from the main Contacts menu to the submenu which comes up from either DMR or DTMF Contact list screens. (Previously New Contact only allowed DMR contacts to be added, but now it is available for DTMF contacts.)
1.2. From the DTMF Contact List submenu, the New Contact and Edit Contact options have been added and will now bring up the new screen for adding or editing the DTMF contact. Currently only digits and * and # are valid DTMF commands. I think that is all that is needed for Allstar, Echolink and IRLP anyway but if the need arises for A through D, we will cross that bridge.
1.3. Delete contact has also been enabled for the DTMF contact submenu.
1.4. If either Contact List is empty when you invoke the submenu, only the New Contact option is available. Note too that you can quickly get to the DTMF contact list in analog mode by pressing sk2+#. When you go to the DTMF contact list from sk2+#, the green button transmits the DTMF code immediately, rather than bringing up the submenu.
2. New Voice Prompts have been added: Note that this version requires a new voice prompt file to be uploaded to your radio. If you do not do this, the DTMF contact screen will not speak correctly.
2.1. Added Name prompt to Channel Details Screen. (Previously you would not hear anything on the name field if it was empty.)
2.2. Added Vox prompt to Channel Details Screen. (Previously Vox was spelled out.)
2.3. Added Name prompt to DMR Contact screen. (Previously it was empty.)
2.4. When Display Options/Radio Information is set to TS, Power or Both, when arrowing up and down in Channel mode, if a channel has a TS or Power override enabled, these will be announced with a pause between them rather than the numbers being run together.
2.5. Fixed the voice prompt for DMR Contact List, which used to be pronounced, dmar Contact list.
2.6. Fixed character input so that # and * are properly announced.
2.7. Reduced the verbosity of voice prompts on the lock screen at Voice Prompt level 2.
2.8. When both the RX and TX CTCSS or DCS codes are set to the same code, the Summary on sk1 hold no longer reads both codes (since they are the same), it instead omits the RX and TX prefix and just announces the code type and code value.
2.9. Now including all English voices in archive in a folder called "other voices"
3. Last release my intention was that when a new voice prompt file was loaded or settings were updated or reset, the radio would default to voice prompt level 3 rather than level 1. I've fixed this properly this time as last time it only worked in certain circumstances.
Thanks to Jan Hegr, OK1TE, for his assistance with this update. Thanks too to Roger VK3KYY for being on call to answer questions.

29 April 2021 VK7JS
Changes since last release by Roger on 5th April.
1.	Hold down hash while powering on to enable voice prompts if currently disabled. (Useful if sighted and blind person share radio and sighted person leaves radio in unknown state).
2.	After a settings update or when voice prompts are loaded for the first time, voice prompt level will be set to 3 by default rather than 2, as this is better for beginners.
3.	New Voice Prompt Level 2 has been enhanced similar to voice prompt level 3 with follow-on, i.e. at level 3, you’d hear “Channel” followed by name, followed by “mode”, the mode, then “contact” and the contact (if in DMR mode). At level 2 you’ll just hear the name, the mode and the contact if in DMR. So, it is as I said, similar to vp3 with follow on enforced. This should suit many users who like more prompts than level 1 provides, and less than what level 3 provides.
4.	New Channel summary key: Hold sk1 in channel mode. You’ll hear the number (which is essential for when you want to use direct channel entry, i.e. Go To channel), the name, (the mode (at level 3), the frequency, any codes etc on the channel, (or if digital, the timeslot and color code etc) (Thanks to OK1TE). This is similar to the GD77S except the S does not need to announce channel number since you can’t do direct entry on that model since there is no keypad.
5. In VFO mode, when changing between receive and transmit input fields with sk2 + Up or Down arrow, you will now hear which field you are moving to.
6. In Channel mode, changing between zones with Sk2+up/down will announce the zone and channel at level 3 and continue to just announce the channel at level 2.
7. Fixed long hold of * to reset the timeslot to announce the timeslot the channel was reset to.

Plese note: 
Ian Spencer's original package contained both PDF and MP3 files for his documentation. To make this package available via email, all documentation except the user guide has been converted to text files.
The CPS software is also distributed separately. Please request it via email or get it from the web via Ian's link if available.

Joseph Stephen (VK7JS).
joestephen@skymesh.com.au
