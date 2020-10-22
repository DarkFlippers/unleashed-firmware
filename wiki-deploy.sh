rm -rf flipperzero-firmware-community.wiki/*
cp -RL wiki/* flipperzero-firmware-community.wiki/
cp README.md flipperzero-firmware-community.wiki/Home.md
cd flipperzero-firmware-community.wiki && git add * && git commit -a -m "deployed by script" && git push -f
