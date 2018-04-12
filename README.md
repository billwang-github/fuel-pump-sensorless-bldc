# fuel-pump-sensorless-bldc

MCU: HT6FM5440

180412

-- add ISR routine to ISR.c

-- can work with CDROM BLDC at 5V~12V

-- PWM ramp up OK

-- PWM start from duty from 700

180228
--	修改換相時間為0.5 hall timr - delay

180411
--  可正常換相
--  設定起始PWM duty,觀察起動的比較器雜訊,調整hall noise filter參數到比較乾淨

180412a
-- pust ISR routine to isr.c

180412b
-- 重新整理無用的變數及函數宣告

180412d
-- 增加 hardware mask mode及hall sensor delay選項
-- 參數化 PWM及hall noise filter
