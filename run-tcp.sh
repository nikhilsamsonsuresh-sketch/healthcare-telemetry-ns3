#!/bin/bash

for load in 1 4 8
do
    echo ""
    echo "===================================="
    echo " TCP experiment: Background load ${load} Mbps"
    echo "===================================="

    ./ns3 run "scratch/healthcare-wireless/healthcare-wireless --protocol=tcp --backgroundMbps=${load}"

    if [ $? -ne 0 ]; then
        echo "TCP experiment failed at ${load} Mbps"
        exit 1
    fi
done

echo ""
echo "All TCP experiments completed."
