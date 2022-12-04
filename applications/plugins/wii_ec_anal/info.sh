echo "MARKED AS TODO"
echo "=============="
grep //! *.c *.h

echo -e "\nSUPPORTED CONTROLLERS"
echo "====================="
grep '\[PID_.*{ {' wii_ec.c | head -n -3 | sed 's/\s*\(.*\)/\1/'

echo -e "\nLOGGING"
echo "======="
grep LOG_LEVEL *.h | grep -v '#if '
