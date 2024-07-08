#! /bin/bash

### Use "bash GS_Satnet_Simulation.sh" to run this transcript.
if [ -f "SIMOUTPUT_Detailed_forread.txt" ]; then
    rm "SIMOUTPUT_Detailed_forread.txt"
fi

if [ -f "SIMOUTPUT_foruse.txt" ]; then
    rm "SIMOUTPUT_foruse.txt"
fi



runfile_name="MeshNet_connected_ip2_route_trace_goundnet.cc"
total_run_kaisu=50


#Read sat1.txt & sat2.txt
cd ./LinkingLogic_Decision
#----------------------------------------------------------------
#sat1_filename="NSH_sat1.txt"
#sat2_filename="NSH_sat2.txt"
sat1_filename="NSD_sat1.txt"
sat2_filename="NSD_sat2.txt"
#sat1_filename="LRST_sat1.txt"
#sat2_filename="LRST_sat2.txt"
#sat1_filename="MFWFS_sat1.txt"
#sat2_filename="MFWFS_sat2.txt"
#----------------------------------------------------------------
sat1_arr=()
sat2_arr=()
while IFS= read -r line
do
    sat1_arr+=("$line")
done < "$sat1_filename"
while IFS= read -r line
do
    sat2_arr+=("$line")
done < "$sat2_filename"
runtime=()
for ((i=0; i<total_run_kaisu; i++))
do
  runtime+=(30.0)
done

cd ../
for((i=0;i<total_run_kaisu;i++))
do
    ./ns3 run scratch/$runfile_name -- --time=${runtime[i]} --sat1=${sat1_arr[i]} --sat2=${sat2_arr[i]}
    wait
done


#Analysizing output
ana_filename="SIMOUTPUT_foruse.txt"
Packet_Loss_ratio=()
Average_Throughput=()
End_to_End_Delay=()
End_to_End_Jitter=()

while IFS= read -r line
do
    Packet_Loss_ratio+=("$line")
    read -r line
    Average_Throughput+=("$line")
    read -r line
    line="${line%ns}"   
    line=$(printf "%.10f" "$line")  # 使用printf将科学计数法的数字转换为浮点数表示
    End_to_End_Delay+=("$line")
    read -r line
    line="${line%ns}"   
    line=$(printf "%.10f" "$line")
    End_to_End_Jitter+=("$line")
done < "$ana_filename"

ave_PLR=0
ave_AT=0
ave_E2ED=0
ave_E2EJ=0
for ((i=0; i<total_run_kaisu; i++))
do
    # 使用bc进行浮点数除法计算
    ave_PLR=$(echo "${ave_PLR} + ${Packet_Loss_ratio[i]}" | bc -l)
    ave_AT=$(echo "${ave_AT} + ${Average_Throughput[i]}" | bc -l)
    ave_E2ED=$(echo "${ave_E2ED} + ${End_to_End_Delay[i]}" | bc -l)
    #ave_E2EJ=$(echo "${ave_E2EJ} + ${End_to_End_Jitter[i]}" | bc -l)
done
ave_PLR=$(echo "${ave_PLR} / $total_run_kaisu" | bc -l)
ave_AT=$(echo "${ave_AT} / $total_run_kaisu" | bc -l)
ave_E2ED=$(echo "${ave_E2ED} / $total_run_kaisu" | bc -l)
#ave_E2EJ=$(echo "${ave_E2EJ} / $total_run_kaisu" | bc -l)
#由于换路，必然产生jitter。此时jitter不能直接取每一次的平均值，而是最大延迟与最小延迟的时间差
maxDelay=$(echo "${End_to_End_Delay[0]}" | bc -l)
minDelay=$(echo "${End_to_End_Delay[0]}" | bc -l)
for ((i=0; i<total_run_kaisu; i++))
do
    if (( $(echo "${End_to_End_Delay[i]} > $maxDelay" | bc -l) )); then
        maxDelay=${End_to_End_Delay[i]}
    fi
    if (( $(echo "${End_to_End_Delay[i]} < $minDelay" | bc -l) )); then
        minDelay=${End_to_End_Delay[i]}
    fi
done
ave_E2EJ=$(echo "${maxDelay}-${minDelay}" | bc -l)

# 去除末尾零
if [[ "$ave_PLR" != "0" ]]; then
    ave_PLR=$(echo "$ave_PLR" | sed 's/0*$//')
fi
if [[ "$ave_AT" != "0" ]]; then
    ave_AT=$(echo "$ave_AT" | sed 's/0*$//')
fi
if [[ "$ave_E2ED" != "0" ]]; then
    ave_E2ED=$(echo "$ave_E2ED" | sed 's/0*$//')
fi
if [[ "$ave_E2EJ" != "0" ]]; then
    ave_E2EJ=$(echo "$ave_E2EJ" | sed 's/0*$//')
fi
# 将ave_E2ED,ave_E2EJ转换为科学计数法表示
ave_E2ED=$(printf "%.10e" "$ave_E2ED")
ave_E2EJ=$(printf "%.10e" "$ave_E2EJ")

echo -e "\n\n\n"
echo "シミュレーション総回数:$total_run_kaisu。結果はここから書いております。"
echo "=======================>>>>FINAL SIMULATION RESULT<<<<========================="
echo "||   ave_PLR：$ave_PLR"
echo "||   ave_AT:$ave_AT"Kbps
echo "||   ave_E2ED:$ave_E2ED"ns
echo "||   ave_E2EJ:$ave_E2EJ"ns          
echo "||   (maxDelay:$maxDelay ns  minDelay:$minDelay ns)"   
echo "=======================>>>>FINAL SIMULATION RESULT<<<<========================="
echo -e "\n\n\n"

# 保存结果到txt文件
cd ./LWC_RESULT/
#final_result_filename="SIM_RESULT--NSH(100).txt"
final_result_filename="SIM_RESULT--NSD(100).txt"
#final_result_filename="SIM_RESULT--LRST(100).txt"
#final_result_filename="SIM_RESULT--MFWFS(100).txt"

echo "シミュレーション総回数:$total_run_kaisu。結果はここから書いております。" >> $final_result_filename
echo "=======================>>>>FINAL SIMULATION RESULT<<<<=========================" >> $final_result_filename
echo "||   ave_PLR：$ave_PLR" >> $final_result_filename
echo "||   ave_AT:$ave_AT"Kbps >> $final_result_filename
echo "||   ave_E2ED:$ave_E2ED"ns >> $final_result_filename
echo "||   ave_E2EJ:$ave_E2EJ"ns >> $final_result_filename
echo "||   (maxDelay:$maxDelay ns  minDelay:$minDelay ns)" >> $final_result_filename
echo "=======================>>>>FINAL SIMULATION RESULT<<<<=========================" >> $final_result_filename

cd ..
