# Fennec Media Project

Complete multimedia suite with an audio/video player, converter, CD ripper, tag editor, DSP effect host and a visualization host. Supports playing/converting almost all media formats.
Project Page: http://fennec.sourceforge.net

![Screenshot](/misc/sc.png)

## Features


* Supports playing and converting: mpg, mpeg, avi, vob, flv, 3gp, mov, qt, asf, wmv, divx, mkv, mp4, ogm, rm, rmvb, swf, mp4, m2a, m4a, aac, amr, flac, ape, mpc, raw, aif, aiff, wav, wave, au, caf, snd, svx, paf, fap, gsm, nist, ircam, sf, voc, w64, mat4, mat5, mat, xi, pvf, sds, sd2, vox, spx, ogg, wv, ac3, mod, nst, mdz, mdr, m15, s3m, stm, s3z, xm, xmz, it, itz, mtm, 669, ult, wow, far, mdl, okt, dmf, ptm, med, ams, dbm, dsm, umx, amf, psm, mt2, mid, midi, rmi, smf, mp1, mp2, mp3, cda, wma...
* Audio CD ripping with batch tagging.
* Joining multiple media files into a single file
* Highly dynamic plug-in interface, so you can add/remove plug-ins and edit their settings manually.
* Visualizations.
* Multichannel support (up to 16 channels) for both playback and transcoding.
* A customizable 10 band equalizer for each channel.
* High definition audio processing and output (64bit floating point audio processing, 8bit fixed point-32bit floating/fixed point to 64bit floating/fixed output).
* Tag viewing and editing.
* Formatting file name according to tags.
* Group tag editing (copying and pasting selected tags into many files).
* Equalizers on conversion/ripping and joining.
* Volume and gain adjustments on conversion/ripping and joining.
* Group tagging on conversion (could apply some constant tags into every file converted, artist name etc.).
* Date and time editing on conversion/ripping and joining.
* Editable DSP effects: you can add many effects simultaneously into the playing media and change the order of effects (you can add a bass boost and a reverb at the same time, boost bass before/after reverb etc.).
* Skins (Fennec player doesn't have a user interface itself, it depends on plug-ins you can select to display the user interface).
* Color schemes and manual tints for skins.
* Automatic volume fade up/down.
* Can decode multiple files at the same time (you can keep something playing while doing conversion, a new example for this is 'preview' system).
* File association selection with many options (Fennec player comes with an icon library which has an icon for almost every file format).
* High quality icons for file association settings (256x256 pixel 32bit).
* Multilingual user interface (completely UNICODE user interface so everyone can create a simple text file called a language pack to modify user interface language).
* Highly editable global and local shortcut keys interface.
* Fast and customizable media library.
* Windows shell integration (Fennec player places a context menu item group with open/add options).
* Audio file preview (you can listen to a media file using headphones without interrupting the file which is being played; in order to experience this feature, you need to have two sound cards or multi-streaming enabled sound card).
* Video zoom-in, zoom-out, vertical/horizontal scaling.
* Dual subtitle display - you'll be able to select primary and secondary subtitles for video files, preferably in two languages.
* Supports reading over 50 subtitle formats and over 10 playlist formats (with Unicode support).


## Compiling

Older source code archives can be downloaded from SourceForge project page, the git repository is created with
an updated version of 2010 source archive (http://prdownloads.sourceforge.net/fennec/fennec_source_july_16th_2010.7z?download).

If you're changing the platform of a single project to x64, make sure to compile all the other projects into the same
platform (the precompiled packages are compliled for 32bit architecture, at the moment).
