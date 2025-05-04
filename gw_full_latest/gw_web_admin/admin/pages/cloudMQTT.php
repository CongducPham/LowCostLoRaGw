<!-- Tab panes -->
																	
	<div class="tab-pane fade" id="cloudMQTT-pills">
		</br>
		<div id="cloudMQTT_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">

			<?php
				$date=date('Y-m-d\TH:i:s');
				echo '<p>Date/Time: ';
				echo $date;
				echo '</p>';												
				ob_start();
				system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 1 'uploading with python.*CloudMQTT.*py' | cut -d '>' -f1"); 
				//system("egrep -a 'uploading with python.*CloudMQTT.*py' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '>' -f1");
				$last_upload=ob_get_contents(); 
				ob_clean();
				echo '<p>';
				if ($last_upload=='') {
					echo '<font color="red"><b>no upload with CloudMQTT.py found</b></font>';					
				}
				else {
					$date=str_replace("T", " ", $date, $count);
					$datetime1 = new DateTime($date);
					$last_upload_1=str_replace("T", " ", $last_upload, $count);
					$datetime2 = new DateTime($last_upload_1);
					$interval = $datetime1->diff($datetime2);				
					echo 'last upload time with CloudMQTT.py: <font color="green"><b>';
					echo $last_upload;
					echo $interval->format(' %mm-%dd-%hh-%imin from current date');
					echo '</b></font>';					
				}	
				echo '</p>';								
			?>                            

		  <div class="table-responsive">
			<table class="table table-striped table-bordered table-hover">
			  <thead></thead>
			 <tbody>
				<tr>
					<td>Enabled
						<?php 
							ob_start(); 
							$server=$key_clouds['mqtt_server'];
							system("wget -q --tries=3 --spider $server", $retval);
							$msg=ob_get_contents(); 
							ob_clean();
							if ($retval == 0) {
								echo ' <font color="green"><b>[server online]</b></font>';					
							}
							else {
								echo ' <font color="red"><b>[server offline]</b></font>';					
							}				 
						?>					
					</td>
					
					<td id="cloudMQTT_status_value"></td>
					
					<td align="right">
					<input type="checkbox" id="mqtt_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if(get_cloud_status($clouds, "python CloudMQTT.py")) echo "checked";?>>
					</td>
			
					<!-- <td align="right"><button id="btn_edit_cloudMQTT_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
					
					<td id="td_edit_cloudMQTT_status">
						<div id="div_cloudMQTT_status_options" class="form-group">
							<div class="radio">
							<fieldset id="cloudMQTT_status_group" >
								<label>
									<input type="radio" name="cloudMQTT_status_group" id="cloudMQTT_true" value="true" checked>True
								</label>
								</br>
								<label>
									<input type="radio" name="cloudMQTT_status_group" id="cloudMQTT_false" value="false" >False
								</label>
								</fieldset>
							</div>
						</div>
					</td> 
					<td id="td_cloudMQTT_status_submit" align="right">
						<button id="btn_cloudMQTT_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
					</td>
				</tr>
				<tr>
					<td>MQTT Server</td>
					<td id="td_cloudMQTT_MQTT_server_value">
						<?php 
							echo $key_clouds['mqtt_server'];
						?>
					</td> 
					<td align="right">
						<button id="btn_edit_cloudMQTT_MQTT_server" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_MQTT_server">
						<div id ="div_cloudMQTT_MQTT_server" class="form-group">
							<label>MQTT server</label>
							<input id="cloudMQTT_MQTT_server_input" class="form-control" placeholder="e.g. test.mosquitto.org" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_MQTT_server_submit" align="right">
						<button id="btn_cloudMQTT_MQTT_server_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>project name</td>
					<td id="td_cloudMQTT_project_name_value">
						<?php 
							echo $key_clouds['mqtt_project_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_project_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_project_name">
						<div id ="div_cloudMQTT_project_name" class="form-group">
							<label>Project name</label>
							<input id="cloudMQTT_project_name_input" class="form-control" placeholder="e.g. waziup" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_project_name_submit" align="right">
						<button id="btn_cloudMQTT_project_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>organization name</td>
					<td id="td_cloudMQTT_organization_name_value">
						<?php 
							echo $key_clouds['mqtt_organization_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_organization_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_organization_name">
						<div id ="div_cloudMQTT_organization_name" class="form-group">
							<label>Organization name</label>
							<input id="cloudMQTT_organization_name_input" class="form-control" placeholder="e.g. UPPA" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_organization_name_submit" align="right">
						<button id="btn_cloudMQTT_organization_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>sensor name</td>
					<td id="td_cloudMQTT_sensor_name_value">
						<?php 
							echo $key_clouds['mqtt_sensor_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_sensor_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_sensor_name">
						<div id ="div_cloudMQTT_sensor_name" class="form-group">
							<label>Sensor name</label>
							<input id="cloudMQTT_sensor_name_input" class="form-control" placeholder="e.g. Sensor" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_sensor_name_submit" align="right">
						<button id="btn_cloudMQTT_sensor_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Indicate a list of accepted device addresses, e.g. 6,7,0x01020304">source list</a></td>
					<td id="td_cloudMQTT_source_list_value">
						<?php 
							$array = $key_clouds['mqtt_source_list'];
							$size = count($key_clouds['mqtt_source_list']);
							$i = 0;
							if($size == 0) echo "Empty";
							foreach( $array as &$cel){
								if(($size -1) == $i ){
									echo $cel ;							
								}else{
									echo $cel . ", ";
								}
								$i++;
							}
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_source_list" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_source_list">
						<div id ="div_cloudMQTT_source_list" class="form-group">
							<label>Add a sensor to your list</label>
							<input id="cloudMQTT_source_list_input" class="form-control" placeholder="e.g. 6,7,0x01020304" type="text" 
							value="<?php 
								$array = $key_clouds['mqtt_source_list'];
								$size = count($key_clouds['mqtt_source_list']);
								$i = 0;
								foreach( $array as &$cel){
									if(($size -1) == $i ){
										echo $cel ;							
									}else{
										echo $cel . ", ";
									}
									$i++;
								}
							?>"
							 autofocus>
						</div>
					</td>
					<td id="td_cloudMQTT_source_list_submit" align="right">
						<button id="btn_cloudMQTT_source_list_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
			 </tbody>
			</table>
					<p>If you want to specify a non standard port, indicate it in the MQTT server field, i.e. <tt>test.mosquitto.org:1234</tt>. Default is 1883.</p>
					<p>The MQTT cloud will build a topic name using the following format: <tt>project_name/organization_name/sensor_name+device_addr/nomenclature</tt>. For instance, sensor 2 hosted by <tt>UPPA</tt> from <tt>waziup</tt> project and sending "<tt>TC/22.5/HU/54</tt>" will publish both <tt>waziup/UPPA/Sensor2/TC</tt> and <tt>waziup/UPPA/Sensor2/HU</tt> topics. You can subscribe to <tt>waziup/UPPA/Sensor2/#</tt> to cover all levels from <tt>waziup/UPPA/Sensor2</tt>.</p>
		
		<p><b>Do not use space</b> in any of these parameters, use '_' or '-' instead.</p>
					
		  </div>
		</div>
</div>		
