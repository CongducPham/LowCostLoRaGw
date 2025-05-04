<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudNoInternet-pills">
		</br>
		<div id="cloudNoInternet_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">
		
			<?php	
				$date=date('Y-m-d\TH:i:s');
				echo '<p>Date/Time: ';
				echo $date;
				echo '</p>';											
				ob_start();
				system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 1 'uploading with python.*CloudNoInternet.*py' | cut -d '>' -f1"); 
				//system("egrep -a 'uploading with python.*CloudNoInternet.*py' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '>' -f1");
				$last_upload=ob_get_contents(); 
				ob_clean();
				echo '<p>';
				if ($last_upload=='') {
					echo '<font color="red"><b>no upload with CloudNoInternet.py found</b></font>';					
				}
				else {
					$date=str_replace("T", " ", $date, $count);
					$datetime1 = new DateTime($date);
					$last_upload_1=str_replace("T", " ", $last_upload, $count);
					$datetime2 = new DateTime($last_upload_1);
					$interval = $datetime1->diff($datetime2);				
					echo 'last upload time with CloudNoInternet.py: <font color="green"><b>';
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
				
				<td id="cloudNoInternet_status_value"></td>
				
				<td align="right">
				<input type="checkbox" id="nointernet_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if(get_cloud_status($clouds, "python CloudNoInternet.py")) echo "checked";?>>
				</td>
			
				<!-- <td align="right"><button id="btn_edit_cloudNoInternet_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
				
				<td id="td_edit_cloudNoInternet_status">
					<div id="div_cloudNoInternet_status_options" class="form-group">
						
						<div class="radio">
						<fieldset id="cloudNoInternet_status_group" >
							<label>
								<input type="radio" name="cloudNoInternet_status_group" id="cloudNoInternet_true" value="true" checked>True
							</label>
							</br>
							<label>
								<input type="radio" name="cloudNoInternet_status_group" id="cloudNoInternet_false" value="false" >False
							</label>
							</fieldset>
						</div>
					</div>
				</td> 
				<td id="td_cloudNoInternet_status_submit" align="right">
					<button id="btn_cloudNoInternet_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			   </tr>
			 </tbody>
			</table>
			<p>Cloud No Internet is used with WAZIUP cloud only.</pp>
		  </div>
		</div>
</div>		
