# Camera-calibration-Circular-calibration-plate 
相机标定之圆形标定板(Camera calibration-Circular calibration plate)   
直接复制到自己的main.cpp文件中即可运行   
需要注意修改的地方：  
1. main()主函数中，File_Diretory1是图片所储存的路劲，注意修改自己的路径   
2. main()主函数中，FileType是图片的类型（后缀，注意所以图片的类型都要相同才可以）   
3. main()主函数中，board_size为标定板的尺寸（水平和垂直的圆的数目）   
4. main()主函数中，square_size为两两之间圆心的距离（单位为mm）  
一旦运行报错：   
1. 可能是路径或者图片类型的问题   
2. 可能是无法识别到标定板或识别不到标定板上的圆心（这种问题可能出现在标定板过于倾斜或者相机拍摄标定板时的曝光太低（注意相机的曝光最好要和正常手机摄像头的曝光差不多）） 
