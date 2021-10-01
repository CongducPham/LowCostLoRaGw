

rm(list=ls())

library(ggplot2)
library(changepoint)
library(caret)
library(lubridate)
library(stringr)
library(MASS)

set.seed(123)

train_set = read.csv('data/Train.csv', stringsAsFactors = F)
test_set  = read.csv('data/SampleSubmission.csv', stringsAsFactors = F)


common_features = c("timestamp","Air.temperature..C." , "Air.humidity...." ,
                    "Pressure..KPa." , "Wind.speed..Km.h.", "Wind.gust..Km.h." ,
                    "Wind.direction..Deg.", "date")



# FEATURES ENGEENERING
{
  
###### GET PEAKS FOR TRAINING SET:
#### generate peak feature for all values:
  
change_points_all1 = c(1,attr(cpt.var(na.omit(train_set$Irrigation.field.1 ),  method = "PELT", class = T), 'cpts'), nrow(train_set))

for(pp in 2:length(change_points_all1)){
  if(!is.na(train_set[change_points_all1[pp],]$Irrigation.field.1))
    if(train_set[change_points_all1[pp],]$Irrigation.field.1==0){
    }else{
      train_set[change_points_all1[pp],'peak1'] =  train_set[change_points_all1[pp],]$Soil.humidity.1
    }
}



change_points_all2 = c(1,attr(cpt.var(na.omit(train_set$Irrigation.field.2 ),  method = "PELT", class = T), 'cpts'), nrow(train_set))
for(pp in 2:length(change_points_all2)){
  if(!is.na(train_set[change_points_all2[pp],]$Irrigation.field.2))
    if(train_set[change_points_all2[pp],]$Irrigation.field.2==0){
    }else{
      train_set[change_points_all2[pp],'peak2'] =  train_set[change_points_all2[pp],]$Soil.humidity.2
    }
}


change_points_all3 = c(1,attr(cpt.var(na.omit(train_set$Irrigation.field.3 ),  method = "PELT", class = T), 'cpts'), nrow(train_set))
for(pp in 2:length(change_points_all3)){
  if(!is.na(train_set[change_points_all3[pp],]$Irrigation.field.3))
    if(train_set[change_points_all3[pp],]$Irrigation.field.3==0){
    }else{
      train_set[change_points_all3[pp],'peak3'] =  train_set[change_points_all3[pp],]$Soil.humidity.3
    }
}


change_points_all4 = c(1,attr(cpt.var(na.omit(train_set$Irrigation.field.4 ),  method = "PELT", class = T), 'cpts'), nrow(train_set))
for(pp in 2:length(change_points_all4)){
  if(!is.na(train_set[change_points_all4[pp],]$Irrigation.field.4))
    if(train_set[change_points_all4[pp],]$Irrigation.field.4==0){
    }else{
      train_set[change_points_all4[pp],'peak4'] =  train_set[change_points_all4[pp],]$Soil.humidity.4
    }
}

# fix first and last values:
train_set[17162,]$peak3 = train_set[17162,]$Soil.humidity.3

train_set[1,]$peak1 = train_set[1,]$Soil.humidity.1
train_set[1,]$peak2 = train_set[1,]$Soil.humidity.2
train_set[1,]$peak3 = train_set[1,]$Soil.humidity.3
train_set[1,]$peak4 = train_set[1,]$Soil.humidity.4


train_set$peak11 = train_set$peak1
train_set$peak22 = train_set$peak2
train_set$peak33 = train_set$peak3
train_set$peak44 = train_set$peak4



lista=  as.numeric(c( rownames(train_set[ !is.na(train_set$peak1),]), nrow(train_set)))
train_set[nrow(train_set), ]$peak1 = train_set[lista[length(lista)-1],]$peak1
for(p in 2:length(lista) ){
  train_set[lista[p-1]:lista[p]-1, 'peak1'] = seq(train_set[lista[p-1] , 'peak1'] , train_set[lista[p] , 'peak1'],  length.out = nrow(train_set[lista[p-1]:lista[p]-1, ])   )
}



lista=  as.numeric(c( rownames(train_set[ !is.na(train_set$peak2),]), nrow(train_set)))
train_set[nrow(train_set), ]$peak2 = train_set[lista[length(lista)-1],]$peak2
for(p in 2:length(lista) ){
  train_set[lista[p-1]:lista[p]-1, 'peak2'] = seq(train_set[lista[p-1] , 'peak2'] , train_set[lista[p] , 'peak2'],  length.out = nrow(train_set[lista[p-1]:lista[p]-1, ])   )
}




lista=  as.numeric(c( rownames(train_set[ !is.na(train_set$peak3),]), nrow(train_set)))
train_set[nrow(train_set), ]$peak3 = train_set[lista[length(lista)-1],]$peak3
for(p in 2:length(lista) ){
  train_set[lista[p-1]:lista[p]-1, 'peak3'] = seq(train_set[lista[p-1] , 'peak3'] , train_set[lista[p] , 'peak3'],  length.out = nrow(train_set[lista[p-1]:lista[p]-1, ])   )
}




lista=  as.numeric(c( rownames(train_set[ !is.na(train_set$peak4),]), nrow(train_set)))
train_set[nrow(train_set), ]$peak4 = train_set[lista[length(lista)-1],]$peak4
for(p in 2:length(lista) ){
  train_set[lista[p-1]:lista[p]-1, 'peak4'] = seq(train_set[lista[p-1] , 'peak4'] , train_set[lista[p] , 'peak4'],  length.out = nrow(train_set[lista[p-1]:lista[p]-1, ])   )
}


##### REPLACE 0 IRRIGATION WITH -1:
train_set$Irrigation.field.1[train_set$Irrigation.field.1==0] = -1
train_set$Irrigation.field.2[train_set$Irrigation.field.2==0] = -1
train_set$Irrigation.field.3[train_set$Irrigation.field.3==0] = -1
train_set$Irrigation.field.4[train_set$Irrigation.field.4==0] = -1


###### GENERATE IRRIGATION MEAN FOR: 12, 24 and 36 previous hours:
##### 12 five minutes * 12 :
number = 12 * 12

train_set$irrigation1_12hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.1,  number, mean, na.rm = TRUE))
train_set$irrigation2_12hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.2,  number, mean, na.rm = TRUE))
train_set$irrigation3_12hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.3,  number, mean, na.rm = TRUE))
train_set$irrigation4_12hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.4,  number, mean, na.rm = TRUE))

number = 12 * 24

train_set$irrigation1_24hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.1,  number, mean, na.rm = TRUE))
train_set$irrigation2_24hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.2,  number, mean, na.rm = TRUE))
train_set$irrigation3_24hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.3,  number, mean, na.rm = TRUE))
train_set$irrigation4_24hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.4,  number, mean, na.rm = TRUE))


number = 12 * 36

train_set$irrigation1_36hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.1,  number, mean, na.rm = TRUE))
train_set$irrigation2_36hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.2,  number, mean, na.rm = TRUE))
train_set$irrigation3_36hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.3,  number, mean, na.rm = TRUE))
train_set$irrigation4_36hours = c(rep(-1,number-1),rollapply(train_set$Irrigation.field.4,  number, mean, na.rm = TRUE))



#################### get testing set :
train_set$date = as_datetime(train_set$timestamp)

train1 = subset(train_set, select=c( common_features,  "Soil.humidity.1" ,   "Irrigation.field.1" ,'peak1','peak11', 'irrigation1_12hours','irrigation1_24hours','irrigation1_36hours'))
train2 = subset(train_set, select=c( common_features,  "Soil.humidity.2" ,   "Irrigation.field.2" ,'peak2','peak22', 'irrigation2_12hours', 'irrigation2_24hours','irrigation2_36hours'))
train3 = subset(train_set, select=c( common_features,  "Soil.humidity.3" ,   "Irrigation.field.3" ,'peak3','peak33' , 'irrigation3_12hours','irrigation3_24hours','irrigation3_36hours'))
train4 = subset(train_set, select=c( common_features,  "Soil.humidity.4" ,   "Irrigation.field.4" ,'peak4','peak44', 'irrigation4_12hours','irrigation4_24hours','irrigation4_36hours'))


train1$ID = paste0(train1$timestamp, ' x Soil humidity 1')
train2$ID = paste0(train1$timestamp, ' x Soil humidity 2')
train3$ID = paste0(train1$timestamp, ' x Soil humidity 3')
train4$ID = paste0(train1$timestamp, ' x Soil humidity 4')

##### generate test IDs:

test_set$date = unlist( lapply(test_set$ID,function(x){ trimws(unlist(str_split(x,'x'))[1]) }))
test_set$date = as_datetime(test_set$date)


test1 = test_set[ str_detect(test_set$ID, "Soil humidity 1") , ]
test2 = test_set[ str_detect(test_set$ID, "Soil humidity 2") , ]
test3 = test_set[ str_detect(test_set$ID, "Soil humidity 3") , ]
test4 = test_set[ str_detect(test_set$ID, "Soil humidity 4") , ]



test1$Irrigation.field.1 = train_set[ train_set$date %in%  test1$date,   ]$Irrigation.field.1
test2$Irrigation.field.2 = train_set[ train_set$date %in%  test2$date,   ]$Irrigation.field.2
test3$Irrigation.field.3 = train_set[ train_set$date %in%  test3$date,   ]$Irrigation.field.3
test4$Irrigation.field.4 = train_set[ train_set$date %in%  test4$date,   ]$Irrigation.field.4



trainDF1 = train1[  (! train1$ID %in% test_set$ID ) &  train1$date <  min(test1$date) ,]
trainDF2 = train2[  (! train2$ID %in% test_set$ID ) &  train2$date <  min(test2$date) ,]
trainDF3 = train3[  (! train3$ID %in% test_set$ID ) &  train3$date <  min(test3$date) ,]
trainDF4 = train4[  (! train4$ID %in% test_set$ID ) &  train4$date <  min(test4$date) ,]

testDF1 = train1[  (train1$ID %in% test_set$ID ), ]
testDF2 = train2[  (train2$ID %in% test_set$ID ), ]
testDF3 = train3[  (train3$ID %in% test_set$ID ), ]
testDF4 = train4[  (train4$ID %in% test_set$ID ), ]



rm("train1")
rm("train2")
rm("train3")
rm("train4")

rm("test1")
rm("test2")
rm("test3")
rm("test4")

############## estimate humidity changes lag:

estimate_lag = function(range_of_values){
  xmax = range_of_values[length(range_of_values)]
  xmin = range_of_values[1] 
  thelag =  ( xmax - xmin) / (length(range_of_values)-1)
  return(thelag)
}

tmp=cpt.var(trainDF1$Irrigation.field.1 ,  method = "PELT", class = T)
change_points1 = c(1,attr(tmp, 'cpts'), nrow(trainDF1))
for(p1 in 2:length(change_points1)){
  trainDF1[change_points1[p1-1]:change_points1[p1],'estimate_lag1'] =  estimate_lag(trainDF1[change_points1[p1-1]:change_points1[p1],'Soil.humidity.1'] )
}
trainDF1[nrow(trainDF1),]$estimate_lag1 = trainDF1[nrow(trainDF1)-1,]$estimate_lag1


tmp=cpt.var(trainDF2$Irrigation.field.2 ,  method = "PELT", class = T)
change_points2 = c(1,attr(tmp, 'cpts'), nrow(trainDF2))
for(p1 in 2:length(change_points2)){
  trainDF2[change_points2[p1-1]:change_points2[p1],'estimate_lag2'] =  estimate_lag(trainDF2[change_points2[p1-1]:change_points2[p1],'Soil.humidity.2'] )
  
    }
trainDF2[nrow(trainDF2),]$estimate_lag2 = trainDF2[nrow(trainDF2)-1,]$estimate_lag2



tmp=cpt.var(trainDF3$Irrigation.field.3 ,  method = "PELT", class = T)
change_points3 = c(1,attr(tmp, 'cpts'), nrow(trainDF3))
for(p1 in 2:length(change_points3)){
  trainDF3[change_points3[p1-1]:change_points3[p1],'estimate_lag3'] =  estimate_lag(trainDF3[change_points3[p1-1]:change_points3[p1],'Soil.humidity.3'] )
  
  }
trainDF3[nrow(trainDF3),]$estimate_lag3 = trainDF3[nrow(trainDF3)-1,]$estimate_lag3


tmp=cpt.var(trainDF4$Irrigation.field.4 ,  method = "PELT", class = T)
change_points4 = c(1,attr(tmp, 'cpts'), nrow(trainDF4))
for(p1 in 2:length(change_points4)){

  trainDF4[change_points4[p1-1]:change_points4[p1],'estimate_lag4'] =  estimate_lag(trainDF4[change_points4[p1-1]:change_points4[p1],'Soil.humidity.4'] )
  
  }
trainDF4[nrow(trainDF4),]$estimate_lag4 = trainDF4[nrow(trainDF4)-1,]$estimate_lag4


##################" TEST:
## remplace empty values with -1:
testDF1$Irrigation.field.1[is.na(testDF1$Irrigation.field.1)] = -1
testDF2$Irrigation.field.2[is.na(testDF2$Irrigation.field.2)]= -1
testDF4$Irrigation.field.4[is.na(testDF4$Irrigation.field.4)] = -1


#### ESTIMATE changes for test:

mean_hbot = mean(trainDF1[trainDF1$estimate_lag1 < 0 , ]$estimate_lag1)
mean_tlo3 = mean(trainDF1[trainDF1$estimate_lag1 > 0 , ]$estimate_lag1)

mean_hbot = quantile(trainDF1[trainDF1$estimate_lag1 < 0 , ]$estimate_lag1)[1]
mean_tlo3 = quantile(trainDF1[trainDF1$estimate_lag1 > 0 , ]$estimate_lag1)[5] 


testDF1[testDF1$Irrigation.field.1==1,'estimate_lag1'] = mean_tlo3
testDF1[testDF1$Irrigation.field.1==-1,'estimate_lag1'] = mean_hbot


testDF2[testDF2$Irrigation.field.2==1,'estimate_lag2'] = mean_tlo3
testDF2[testDF2$Irrigation.field.2==-1,'estimate_lag2'] = mean_hbot


testDF3[testDF3$Irrigation.field.3==1,'estimate_lag3'] = mean_tlo3
testDF3[testDF3$Irrigation.field.3==-1,'estimate_lag3'] = mean_hbot


testDF4[testDF4$Irrigation.field.4==1,'estimate_lag4'] = mean_tlo3
testDF4[testDF4$Irrigation.field.4==-1,'estimate_lag4'] = mean_hbot


trainDF1$cumsum = cumsum(trainDF1$estimate_lag1) + trainDF1$Soil.humidity.1[1]
trainDF2$cumsum = cumsum(trainDF2$estimate_lag2) + trainDF2$Soil.humidity.2[1]
trainDF3$cumsum = cumsum(trainDF3$estimate_lag3) + trainDF3$Soil.humidity.3[1]
trainDF4$cumsum = cumsum(trainDF4$estimate_lag4) + trainDF4$Soil.humidity.4[1]


#trainDF1$cumsum = cumsum(trainDF1$estimate_lag1) + trainDF1$Soil.humidity.1[1]
#testDF1$cumsum = cumsum(testDF1$estimate_lag1) + trainDF1[nrow(trainDF1),]$Soil.humidity.1
testDF1$cumsum[1] = trainDF1[nrow(trainDF1),]$Soil.humidity.1
for( i in 2:nrow(testDF1)){
  testDF1$cumsum[i]  = testDF1$cumsum[i-1] + testDF1$estimate_lag1[i]
  if(!is.na(testDF1$peak11[i])){
    testDF1$cumsum[i]  = testDF1$peak11[i]
  }
  
  if( testDF1$cumsum[i]    >  testDF1$peak1[i]   ){
    testDF1$cumsum[i]  = testDF1$peak1[i]
  }
  
  
}



testDF2$cumsum[1] = trainDF2[nrow(trainDF2),]$Soil.humidity.2
for( i in 2:nrow(testDF2)){
  testDF2$cumsum[i]  = testDF2$cumsum[i-1] + testDF2$estimate_lag2[i]
  if(!is.na(testDF2$peak22[i])){
    testDF2$cumsum[i]  = testDF2$peak22[i]
  }
  
  if( testDF2$cumsum[i]    >  testDF2$peak2[i]   ){
    testDF2$cumsum[i]  = testDF2$peak2[i]
  }
  
}

testDF3$cumsum[1] = trainDF3[nrow(trainDF3),]$Soil.humidity.3
for( i in 2:nrow(testDF3)){
  testDF3$cumsum[i]  = testDF3$cumsum[i-1] + testDF3$estimate_lag3[i]
  if(!is.na(testDF3$peak33[i])){
    testDF3$cumsum[i]  = testDF3$peak33[i]
  }
  
  if( testDF3$cumsum[i]    >  testDF3$peak3[i]   ){
    testDF3$cumsum[i]  = testDF3$peak3[i]
  }
  
}

testDF4$cumsum[1] = trainDF4[nrow(trainDF4),]$Soil.humidity.4
for( i in 2:nrow(testDF4)){
  testDF4$cumsum[i]  = testDF4$cumsum[i-1] + testDF4$estimate_lag4[i]
  if(!is.na(testDF4$peak44[i])){
    testDF4$cumsum[i]  = testDF4$peak44[i]
  }
  
  if( testDF4$cumsum[i]    >  testDF4$peak4[i]   ){
    testDF4$cumsum[i]  = testDF4$peak4[i]
  }
  
}


}


###### REGRESSION:

features1 = c('Irrigation.field.1','peak1', 'irrigation1_12hours', 'irrigation1_24hours','irrigation1_36hours'    ,'estimate_lag1','cumsum')
features2 = c('Irrigation.field.2','peak2', 'irrigation2_12hours', 'irrigation2_24hours','irrigation2_36hours'   ,'estimate_lag2','cumsum')
features3 = c('Irrigation.field.3','peak3', 'irrigation3_12hours', 'irrigation3_24hours','irrigation3_36hours'   ,'estimate_lag3','cumsum')
features4 = c('Irrigation.field.4','peak4', 'irrigation4_12hours', 'irrigation4_24hours','irrigation4_36hours'   ,'estimate_lag4','cumsum')


model1 =train(x =  trainDF1[ ,c(features1)] ,  trainDF1$Soil.humidity.1 , method="rlm"
              , metric = "RMSE", maximize = F)

model2 =train(x =  trainDF2[ ,c(features2)] , y = trainDF2$Soil.humidity.2 , method="rlm"
              , metric = "RMSE", maximize = F)


model3 =train(x =  trainDF3[ ,c(features3)] , y = trainDF3$Soil.humidity.3 , method="rlm"
              , metric = "RMSE", maximize = F)


model4 =train(x =  trainDF4[ ,c(features4)] , y = trainDF4$Soil.humidity.4 , method="rlm"
              , metric = "RMSE", maximize = F)


preds1 = predict(model1, testDF1[,features1])
preds2 = predict(model2, testDF2[,features2])
preds3 = predict(model3, testDF3[,features3])
preds4 = predict(model4, testDF4[,features4])


all3 = c(preds1, preds2, preds3, preds4)


submission = read.csv('data/SampleSubmission.csv')
submission$Values = all3

write.csv(submission[,c('ID','Values')], file = "daoudi.csv", quote = F, row.names = F)


