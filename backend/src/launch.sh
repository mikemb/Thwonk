#/bin/sh

killall msgdelivery
killall rulerunner

./msgdelivery &
./msgdelivery &
./msgdelivery &
./rulerunner &
./rulerunner &
