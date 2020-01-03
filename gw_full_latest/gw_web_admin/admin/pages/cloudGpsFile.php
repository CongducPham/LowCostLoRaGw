<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudGpsFile-pills">
		</br>
		<div id="cloudGpsFile_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">

			<?php
				$date=date('Y-m-d\TH:i:s');
				echo '<p>Date/Time: ';
				echo $date;
				echo '</p>';												
				ob_start();
				system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 1 'uploading with python.*CloudGpsFile.*py' | cut -d '>' -f1"); 
				//system("egrep -a 'uploading with python.*CloudGpsFile.*py' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '>' -f1");
				$last_upload=ob_get_contents(); 
				ob_clean();
				echo '<p>';
				if ($last_upload=='') {
					echo '<font color="red"><b>no upload with CloudGpsFile.py found</b></font>';					
				}
				else {
					$date=str_replace("T", " ", $date, $count);
					$datetime1 = new DateTime($date);
					$last_upload_1=str_replace("T", " ", $last_upload, $count);
					$datetime2 = new DateTime($last_upload_1);
					$interval = $datetime1->diff($datetime2);				
					echo 'last upload time with CloudGpsFile.py: <font color="green"><b>';
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
				<td>Enabled</td>
				
				<td id="cloudGpsFile_status_value"></td>
				
				<td align="right">
				<input type="checkbox" id="gpsfile_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if(get_cloud_status($clouds, "python CloudGpsFile.py")) echo "checked";?>>
				</td>
			
				<!-- <td align="right"><button id="btn_edit_cloudGpsFile_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
				
				<td id="td_edit_cloudGpsFile_status">
					<div id="div_cloudGpsFile_status_options" class="form-group">					
						<div class="radio">
						<fieldset id="cloudGpsFile_status_group" >
							<label>
								<input type="radio" name="cloudGpsFile_status_group" id="cloudGpsFile_true" value="true" checked>True
							</label>
							</br>
							<label>
								<input type="radio" name="cloudGpsFile_status_group" id="cloudGpsFile_false" value="false" >False
							</label>
							</fieldset>
						</div>
					</div>
				</td> 
				<td id="td_cloudGpsFile_status_submit" align="right">
					<button id="btn_cloudGpsFile_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			 </tr>
			<tr>
				<td>project name</td>
				<td id="cloudGpsFile_project_name_value"><?php echo $key_clouds['gpsfile_project_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_project_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_project_name">
					<div id="div_cloudGpsFile_project_name" class="form-group">
					    		<label>Project name</label>
                                			<input id="cloudGpsFile_project_name_input" class="form-control" placeholder="e.g. waziup" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_project_name_submit" align="right">
					<button id="btn_cloudGpsFile_project_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>organization name</td>
				<td id="cloudGpsFile_organization_name_value"><?php echo $key_clouds['gpsfile_organization_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_organization_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_organization_name">
					<div id="div_cloudGpsFile_organization_name" class="form-group">
					    		<label>Organization name</label>
                                			<input id="cloudGpsFile_organization_name_input" class="form-control" placeholder="e.g. UPPA" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_organization_name_submit" align="right">
					<button id="btn_cloudGpsFile_organization_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>			
			</tr>
			<tr>
				<td>sensor name</td>
				<td id="cloudGpsFile_sensor_name_value"><?php echo $key_clouds['gpsfile_sensor_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_sensor_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_sensor_name">
					<div id="div_cloudGpsFile_sensor_name" class="form-group">
					    		<label>Sensor name</label>
                                			<input id="cloudGpsFile_sensor_name_input" class="form-control" placeholder="e.g. Sensor" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_sensor_name_submit" align="right">
					<button id="btn_cloudGpsFile_sensor_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>			
			</tr>
			<tr>
				<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Indicate a list of accepted device addresses, e.g. 6,7,0x01020304">source list</a></td>
				<td id="cloudGpsFile_source_list_value">
					<?php 
						$array = $key_clouds['gpsfile_source_list'];
						$size = count($key_clouds['gpsfile_source_list']);
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
				<td align="right"><button id="btn_edit_cloudGpsFile_source_list" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_source_list">
					<div id="div_cloudGpsFile_source_list" class="form-group">
					    		<label>Add a sensor to your list</label>
                                			<input id="cloudGpsFile_source_list_input" class="form-control" placeholder="e.g. 6,7,0x01020304" type="text" 
							value="<?php 
								$array = $key_clouds['gpsfile_source_list'];
								$size = count($key_clouds['gpsfile_source_list']);
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
				<td id="td_cloudGpsFile_source_list_submit" align="right">
					<button id="btn_cloudGpsFile_source_list_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>active interval</td>
				<td id="cloudGpsFile_active_interval_value"><?php echo $key_clouds['gpsfile_active_interval_minute']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_active_interval" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_active_interval">
					<div id="div_cloudGpsFile_active_interval" class="form-group">
					    		<label>Interval in minutes</label>
                                			<input id="cloudGpsFile_active_interval_input" class="form-control" placeholder="20" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_active_interval_submit" align="right">
					<button id="btn_cloudGpsFile_active_interval_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>SMS</td>
				<td id="cloudGpsFile_sms_value">
				<?php
					 if( $key_clouds['gpsfile_sms'] == 1){
						echo "Enabled";
					} else{
						echo "Disabled";				
					}
				?>
				</td>
				<td align="right"><button id="btn_edit_cloudGpsFile_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_sms">
					<div id="div_cloudGpsFile_sms" class="form-group">
						<div class="radio">
						<fieldset id="cloudGpsFile_sms_group" >					
							<label>
								<input type="radio" name="cloudGpsFile_sms_group" id="cloudGpsFile_sms_true" value="true" <?php if($key_clouds['gpsfile_sms'] == 1) echo "checked"?> >Enable
							</label>
							</br>
							<label>
								<input type="radio" name="cloudGpsFile_sms_group" id="cloudGpsFile_sms_false" value="false" <?php if($key_clouds['gpsfile_sms'] == 0) echo "checked"?> >Disable
							</label>
							</fieldset>
						</div>							
					</div>
				</td>
				<td id="td_cloudGpsFile_sms_submit" align="right">
					<button id="btn_cloudGpsFile_sms_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>Pin code</td>
				<td id="cloudGpsFile_pin_value"><?php echo $key_clouds['gpsfile_pin']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_pin" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_pin">
					<div id="div_cloudGpsFile_pin" class="form-group">
					    		<label>Pin code</label>
                                			<input id="cloudGpsFile_pin_input" class="form-control" placeholder="Write pin code" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_pin_submit" align="right">
					<button id="btn_cloudGpsFile_pin_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>Contacts</td>
				<td id="cloudGpsFile_contacts_value">
					<?php 
						$array = $key_clouds['gpsfile_contacts'];
						$size = count($key_clouds['gpsfile_contacts']);
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
				<td align="right"><button id="btn_edit_cloudGpsFile_contacts" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_contacts">
					<div id="div_cloudGpsFile_contacts" class="form-group">
					    		<label>Add a mobile phone number your list</label>
                                			<input id="cloudGpsFile_contacts_input" class="form-control" placeholder="e.g. +33645783476" type="text" 
							value="<?php 						
										$array = $key_clouds['gpsfile_contacts'];
										$size = count($key_clouds['gpsfile_contacts']);
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
					 				?>" 
							autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_contacts_submit" align="right">
					<button id="btn_cloudGpsFile_contacts_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>

			 </tbody>
			</table>
			<p>The sensor name will be <tt>project_name+'_'+organization_name+'_'+sensor_name+device_addr</tt>. For instance, for sensor 2 hosted by UPPA from waziup project, the sensor name will be <tt>waziup_UPPA_Sensor2</tt>.</p>
			
			<p><tt>active_interval</tt> defines the time window to mark remote GPS devices as active. It is expressed in minutes.</p>
			
			<p><b>Do not use space</b> in any of these parameters, use '_' or '-' instead.</p>
		  </div>
		</div>
</div>		
