# MangaCast Music Assistant / Sendspin

Le firmware principal réunit maintenant les deux sorties audio :

- `manga_music.yaml` : sortie analogique I²S historique du MangaCast
  (`LRCLK GPIO25`, `BCLK GPIO5`, `DOUT GPIO26`) et sortie optique S/PDIF sur
  `GPIO04` à 48 kHz stéréo.
- `manga_music_spdif.yaml` reste disponible comme variante SPDIF autonome.

Les configurations nécessitent ESPHome 2026.5.0 ou plus récent. Depuis cette
version, Sendspin, `speaker_source`, `audio_file`, le mixer, le resampler et le
mode S/PDIF sont intégrés à ESPHome. Le firmware principal charge le petit
composant local `audio_output_router`, qui permet de choisir la sortie sans
dupliquer la chaîne Sendspin.

## Sélection de la sortie

Dans Home Assistant, ouvrir **Paramètres > Appareils et services > ESPHome**, puis
la MangaCast. L'interrupteur de configuration **SPDIF** sélectionne la sortie :

- désactivé : sortie analogique ;
- activé : sortie optique SPDIF.

Le choix est conservé après un redémarrage. La commutation pendant une lecture
est possible, avec une brève interruption au moment du basculement.

## Music Assistant / Sendspin

Sendspin découvre Music Assistant par mDNS. Le multicast mDNS doit donc circuler
entre le MangaCast et le serveur Music Assistant, et le port TCP 8928 doit être
accessible. Dans Music Assistant, sélectionner le lecteur MangaCast découvert
par le fournisseur Sendspin.

L'écran utilise directement les métadonnées Sendspin. Il affiche, de haut en bas :

1. état de lecture ;
2. titre ;
3. artiste ;
4. album ;
5. volume du groupe en petits caractères.

Les textes trop longs défilent. L'encodeur modifie le volume du groupe Sendspin
par pas de 2 %, et le bouton lecture/pause contrôle également ce groupe.

## Validation et compilation

```bash
python3 -m esphome config manga_music.yaml
python3 -m esphome compile manga_music.yaml

# Variante SPDIF autonome, uniquement si elle est encore souhaitée :
python3 -m esphome config manga_music_spdif.yaml
python3 -m esphome compile manga_music_spdif.yaml
```

Pour installer par USB :

```bash
python3 -m esphome run manga_music.yaml --device /dev/ttyUSB0
# Variante SPDIF autonome :
python3 -m esphome run manga_music_spdif.yaml --device /dev/ttyUSB0
```

La variante SPDIF autonome conserve l'OTA ESPHome, mais pas la mise à jour HTTP
publique. Le firmware principal peut être publié pour les deux modes, puisque le
choix de sortie est désormais effectué après l'installation.

## Publication du firmware principal

1. Modifier la version du projet dans `manga_music.yaml`.
2. Compiler le firmware.
3. Copier `.esphome/build/manga-music/build/firmware.ota.bin` vers
   `update_firmware.bin`.
4. Calculer `md5sum update_firmware.bin`.
5. Reporter la version et le MD5 dans `manifest_update.json`.
