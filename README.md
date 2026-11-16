# RackForge — studio à rack virtuel (VST3 / AU)

Instrument virtuel « tout-en-un » dans l'esprit d'un studio à rack : un synthé, une boîte
à rythmes avec séquenceur 16 pas synchronisé sur l'hôte, une chaîne d'effets et une table
de mixage, dans un seul plugin. Conçu pour **Cubase 9.5 sous Windows 10 (VST3 64 bits)**,
et compilable aussi en AU/VST3 pour macOS.

## Le rack (v0.4)

| Module | Rôle |
|---|---|
| **VEKTOR** | Synthé polyphonique 8 voix : 2 oscillateurs (scie / carré / triangle / sinus, désaccord, octave), sub, filtre passe-bas résonant, enveloppes ADSR ampli + filtre, LFO (hauteur / cutoff), 7 presets |
| **PULSE-8** | Boîte à rythmes 8 sons synthétisés (kick, snare, charleys, clap, toms, rim) — niveau / accord / decay par piste, grille 16 pas, swing, **4 motifs A/B/C/D** (boutons A-D, menu *Copier…* / effacer, mode **Enchaîner** qui joue A→B→C→D une mesure chacun), suit le transport de Cubase ; les pads se jouent aussi en MIDI sur le **canal 10** (notes 36 à 43) |
| **PADLAB** | Sampler 16 pads : chargez **vos propres fichiers** WAV / AIFF / FLAC / OGG / MP3 (bouton *Charger…* ou glisser-déposer sur un pad, jusqu'à 60 s par pad). Par pad : niveau, accord ±24 demi-tons, point de départ / fin, boucle, one-shot, attaque, relâchement, filtre, pan. Mode **chromatique** : le clavier joue le pad sélectionné sur toute la tessiture (do central = hauteur d'origine). Pads jouables en MIDI sur le **canal 2** (notes 48 à 63). **REC** : enregistre l'entrée audio directement dans le pad sélectionné (60 s max, silence de début rogné, normalisation). **Slices** : découpe le son du pad sélectionné sur ses transitoires (ou en 4/8/16 parts égales) et répartit les tranches sur les pads suivants. Les chemins des fichiers sont enregistrés dans le projet ; enregistrements et tranches sont exportés en WAV dans `Documents\RackForge\Samples` à la sauvegarde |
| **GRIT** | Distorsion à saturation douce (drive, tonalité, mix) |
| **CHORUS** | Chorus stéréo (vitesse, profondeur, délai, feedback, mix) |
| **PHASER** | Phaser 6 étages (vitesse, profondeur, fréquence centrale, feedback, mix) |
| **EQ-3** | Égaliseur 3 bandes : graves (shelf), médiums (cloche avec Q), aigus (shelf), ±15 dB |
| **ECHO-T** | Delay synchronisé au tempo (1/4, 1/8, pointé, triolet, 1/16…) |
| **HALL** | Réverbération (taille, amortissement, largeur, mix) |
| **SQUASH** | Compresseur (seuil, ratio, attaque, relâchement, gain) |
| **MIX-4** | Faders, panoramiques, mutes et vu-mètres pour le synthé, la boîte à rythmes, le sampler et le master |

Routage MIDI : **canal 2 → sampler PADLAB**, **canal 10 → boîte à rythmes PULSE-8**, tous les autres canaux → synthé VEKTOR.
Dans Cubase, créez une piste MIDI par module, dirigée vers RackForge, et réglez son canal (2 pour le sampler, 10 pour les pads de batterie).
Le séquenceur démarre avec la lecture de Cubase (bouton **RUN** allumé). Dans la version
Standalone, sans transport hôte, il tourne librement à 120 BPM.

## Rack à modules libres
Le rack est libre : chaque module a des boutons **^** (monter), **v** (descendre) et **x** (retirer)
dans sa barre de titre, et le bouton **+ Ajouter un module** en bas du rack propose les
instruments et effets absents. **L'ordre des effets dans le rack est l'ordre de traitement audio**
(ex. EQ avant la distorsion, ou reverb avant le compresseur). Un instrument retiré ne joue plus
(ses réglages et ses échantillons sont conservés si vous le remettez). La composition du rack est
enregistrée dans le projet Cubase. Par défaut : VEKTOR, PULSE-8, PADLAB, GRIT, ECHO-T, HALL, SQUASH
(CHORUS, PHASER et EQ-3 sont à ajouter).

## Enregistrer dans un pad (REC)
- **Standalone** : Options › Audio/MIDI → choisir l'entrée de votre interface audio, puis REC.
- **Cubase** : RackForge expose une entrée audio optionnelle (side-chain). Si votre version de Cubase
  permet d'activer le side-chain sur un instrument VST3, routez-y une piste audio puis REC. Sinon,
  exportez l'audio en WAV (Fichier › Exporter › Mixage audio) et glissez le fichier sur un pad :
  le résultat est identique.

## Compilation

### Automatique (GitHub Actions — recommandé)
1. Créer un dépôt GitHub (public) et y pousser ce dossier.
2. Onglet **Actions** → le workflow « Build RackForge » se lance seul à chaque push
   (ou via « Run workflow »).
3. Télécharger les artefacts : `RackForge-Windows-VST3` (pour Cubase) et, pour le Mac,
   `RackForge-macOS-AU` / `RackForge-macOS-VST3`.

### Manuelle sous Windows 10
Prérequis : Visual Studio 2022 (Desktop C++), CMake ≥ 3.22, Git.
```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target RackForge_VST3
```
Le plugin est produit dans `build\RackForge_artefacts\Release\VST3\RackForge.vst3`.
JUCE est téléchargé automatiquement par CMake au premier `configure`.

## Installation dans Cubase 9.5 (Windows 10)
1. Copier le dossier `RackForge.vst3` dans `C:\Program Files\Common Files\VST3\`.
2. Lancer Cubase → **Studio › Gestionnaire de plug-ins VST** → bouton **Rafraîchir**.
3. Ajouter une **piste Instrument** avec « RackForge » (catégorie Instrument / Synth).
4. Appuyer sur Lecture : la boîte à rythmes suit le tempo du projet ; jouer au clavier
   pour le synthé. Pour jouer les pads en direct, régler une piste MIDI sur le canal 10.

Sur macOS : `RackForge.component` dans `~/Library/Audio/Plug-Ins/Components/`
(GarageBand / Logic) et `RackForge.vst3` dans `~/Library/Audio/Plug-Ins/VST3/`.

## Structure du code
```
CMakeLists.txt              projet CMake (JUCE 8 via FetchContent)
Source/Params.h             paramètres du rack (APVTS)
Source/SynthDevice.h        synthé VEKTOR (oscillateurs PolyBLEP, filtre, enveloppes, LFO)
Source/DrumDevice.h         boîte à rythmes PULSE-8 (synthèse + séquenceur)
Source/SamplerDevice.h      sampler PADLAB (chargement de fichiers, 24 voix, boucle, chromatique)
Source/Effects.h            GRIT, CHORUS, PHASER, EQ-3, ECHO-T, HALL, SQUASH
Source/PluginProcessor.*    moteur audio, routage MIDI, mixeur, état
Source/PluginEditor.*       interface rack (panneaux, grille de pas, mixeur)
.github/workflows/build.yml compilation Windows + macOS
```

## Idées pour la suite
- Vue arrière du rack avec câblage libre (envois d'effets par instrument)
- Export des pads en kit, chaînes de motifs personnalisées
- Export / import de presets, motifs de batterie multiples (A/B/C/D)
- Second synthé (tables d'ondes)
