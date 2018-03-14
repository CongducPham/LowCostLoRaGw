<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudNodeRed-pills">
		</br>
		<div id="cloudNodeRed_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">
		  <div class="table-responsive">
			<table class="table table-striped table-bordered table-hover">
			  <thead></thead>
			 <tbody>
			   <tr>
				<td>Enabled</td>
				<td id="cloudNodeRed_status_value"><?php cloud_status($clouds, "python CloudNodeRed.py"); ?></td>
				<td align="right"><button id="btn_edit_cloudNodeRed_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudNodeRed_status">
					<div id="div_cloudNodeRed_status_options" class="form-group">
						
						<div class="radio">
							<label>
								<input type="radio" name="optionsRadios" id="cloudNodeRed_true" value="true" checked>True
							</label>
							</br>
							<label>
								<input type="radio" name="optionsRadios" id="cloudNodeRed_false" value="false" >False
							</label>
						</div>
					</div>
				</td> 
				<td id="td_cloudNodeRed_status_submit" align="right">
					<button id="btn_cloudNodeRed_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			   </tr>
			   <!--
			   <tr>
				<td>Write Key</td>
				<td id="write_key_value"><?php echo $key_clouds['thingspeak_channel_key']; ?></td>
				<td align="right"><button id="btn_edit_write_key" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_write_key">
					<div id="div_update_write_key" class="form-group">
						<label>Write Key</label>
						<input id="write_key_input" class="form-control" placeholder="Write key" name="write_key" type="text" value="" autofocus>
					</div>
				</td> 
				<td id="td_write_key_submit" align="right">
						<button id="write_key_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			   </tr>
			   -->
			 </tbody>
			</table>
		  </div>
		</div>
</div>		
