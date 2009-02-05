(*
 * $Id$
 *
 * Multi aircrafts receiver, logger and broadcaster
 *  
 * Copyright (C) 2004 CENA/ENAC, Pascal Brisset, Antoine Drouin
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 *
 *)

let my_id = "ground"
let gps_mode_3D = 3

open Printf
open Latlong
open Aircraft
module U = Unix
module LL = Latlong

module Ground = struct let name = "ground" end
module Ground_Pprz = Pprz.Messages(Ground)
module Alerts_Pprz = Pprz.Messages(struct let name = "alert" end)
module Dl_Pprz = Pprz.Messages (struct let name = "datalink" end)



let (//) = Filename.concat
let logs_path = Env.paparazzi_home // "var" // "logs"
let conf_xml = Xml.parse_file (Env.paparazzi_home // "conf" // "conf.xml")
let srtm_path = Env.paparazzi_home // "data" // "srtm"

let kml = ref false
let hostname = ref "localhost"

let rec norm_course =
  let _2pi = 2. *. LL.pi in
  fun c ->
    if c < 0. then norm_course (c +. _2pi)
    else if c >= _2pi then norm_course (c -. _2pi)
    else c

(** FIXME: Should be read from messages.xml *)
let ap_modes = [|"MANUAL";"AUTO1";"AUTO2";"HOME";"NOGPS"|]
let _AUTO2 = 2
let gaz_modes = [|"MANUAL";"GAZ";"CLIMB";"ALT"|]
let lat_modes = [|"MANUAL";"ROLL_RATE";"ROLL";"COURSE"|]
let gps_modes = [|"NOFIX";"DRO";"2D";"3D";"GPSDRO"|]
let gps_hybrid_modes = [|"OFF";"ON"|]
let horiz_modes = [|"WAYPOINT";"ROUTE";"CIRCLE"|]

let check_index = fun i t where ->
  if i < 0 || i >= Array.length t then begin
    Debug.call 'E' (fun f -> fprintf f "Wrong index in %s: %d" where i);
    -1
  end else
    i

let get_indexed_value = fun t i ->
  if i >= 0 then t.(i) else "UNK"  

(** The aircrafts store *)
let aircrafts = Hashtbl.create 3

(** Broadcast of the received aircrafts *)
let aircraft_msg_period = 500 (* ms *)
let wind_msg_period = 5000 (* ms *)
let aircraft_alerts_period = 1000 (* ms *)
let send_aircrafts_msg = fun _asker _values ->
  assert(_values = []);
  let names = String.concat "," (Hashtbl.fold (fun k _v r -> k::r) aircrafts []) ^ "," in
  ["ac_list", Pprz.String names]

let make_element = fun t a c -> Xml.Element (t,a,c)

let expand_ac_xml = fun ac_conf ->
  let prefix = fun s -> sprintf "%s/conf/%s" Env.paparazzi_home s in
  let parse = fun a ->
    let file = prefix (ExtXml.attrib ac_conf a) in
    try
      Xml.parse_file file
    with
      Xml.File_not_found _ ->
	prerr_endline (sprintf "File not found: %s" file);
	make_element "file_not_found" ["file",a] []
    | Xml.Error e ->
	let s = Xml.error e in
	prerr_endline (sprintf "Parse error in %s: %s" file s);
	make_element "cannot_parse" ["file",file;"error", s] [] in
  let fp = parse "flight_plan" in
  let af = parse "airframe" in
  let rc = parse "radio" in
  let children = Xml.children ac_conf@[fp; af; rc] in
  make_element (Xml.tag ac_conf) (Xml.attribs ac_conf) children
  
let log_xml = fun timeofday data_file ->
  let conf_children = 
    List.map
      (fun x ->	  if Xml.tag x = "aircraft" then expand_ac_xml x else x)
      (Xml.children conf_xml) in
  let expanded_conf = make_element (Xml.tag conf_xml) (Xml.attribs conf_xml) conf_children in
  make_element 
    "configuration"
    ["time_of_day", string_of_float timeofday; "data_file", data_file]
    [expanded_conf; Pprz.messages_xml ()]
			 
  
let start_time = U.gettimeofday ()

(* Opens the log files *)
let logger = fun () ->
  let d = U.localtime start_time in
  let basename = sprintf "%02d_%02d_%02d__%02d_%02d_%02d" (d.U.tm_year mod 100) (d.U.tm_mon+1) (d.U.tm_mday) (d.U.tm_hour) (d.U.tm_min) (d.U.tm_sec) in
  if not (Sys.file_exists logs_path) then begin
    printf "Creating '%s'\n" logs_path; flush stdout;
    Unix.mkdir logs_path 0o755
  end;
  let log_name = sprintf "%s.log" basename
  and data_name = sprintf "%s.data" basename in
  let f = open_out (logs_path // log_name) in
  output_string f (Xml.to_string_fmt (log_xml start_time data_name));
  close_out f;
  open_out (logs_path // data_name)

let fvalue = fun x ->
  match x with
    Pprz.Float x -> x 
    | Pprz.Int32 x -> Int32.to_float x 
    | Pprz.Int x -> float_of_int x 
    | _ -> failwith (sprintf "Receive.log_and_parse: float expected, got '%s'" (Pprz.string_of_value x))

	  


let ivalue = fun x ->
  match x with
    Pprz.Int x -> x
  | Pprz.Int32 x -> Int32.to_int x 
  | _ -> failwith "Receive.log_and_parse: int expected"

let print_xml = fun ac_name file xml ->
  let f = open_out (sprintf "%s/var/%s/%s" Env.paparazzi_home ac_name file) in
  fprintf f "%s\n" (Xml.to_string_fmt xml);
  close_out f


let update_waypoint = fun ac wp_id p alt ->
  let new_wp = { altitude = alt; wp_utm = p } in
  try
    let prev_wp = Hashtbl.find ac.waypoints wp_id in
    if new_wp <> prev_wp then
      Hashtbl.replace ac.waypoints wp_id new_wp
  with
    Not_found ->
      Hashtbl.add ac.waypoints wp_id new_wp



let kml_update_waypoints =
  let last_state = ref [] in
  fun ac ->
    let l = ref [] in
    Hashtbl.iter (fun wp_id wp -> if wp_id > 0 then l := (wp_id, wp) :: !l) ac.waypoints;
    if !l <> !last_state then begin
      last_state := !l;
      let url_flight_plan = sprintf "http://%s:8889/var/%s/flight_plan.kml" !hostname ac.name in
      let changes = List.map (fun (wp_id, wp) -> Kml.change_waypoint ac.name wp_id(of_utm WGS84 wp.wp_utm) wp.altitude) !l in
      let kml_update = Kml.link_update url_flight_plan changes in
      print_xml ac.name "wp_changes.kml" kml_update
    end
  

let kml_update_horiz_mode =
  let last_horiz_mode = ref UnknownHorizMode in
  fun ac ->
    if ac.horiz_mode <> !last_horiz_mode then begin
      last_horiz_mode := ac.horiz_mode;
      let url_flight_plan = sprintf "http://%s:8889/var/%s/flight_plan.kml" !hostname ac.name in
      let alt = ac.desired_altitude in
      match ac.horiz_mode with
	Segment (p1, p2) -> 
	  let coordinates = String.concat " " (List.map (fun p -> Kml.coordinates (of_utm WGS84 p) alt) [p1; p2]) in
	  let kml_changes = Kml.update_linear_ring url_flight_plan "horiz_mode" coordinates in
	  print_xml ac.name "route_changes.kml" kml_changes
      | Circle (p, r) ->
	  let coordinates = Kml.circle p (float r) alt in
	  let kml_changes = Kml.update_linear_ring url_flight_plan "horiz_mode" coordinates in
	  print_xml ac.name "route_changes.kml" kml_changes
      | _ -> ()
    end


let format_string_field = fun s ->
  let s = String.copy s in
  for i = 0 to String.length s - 1 do
    match s.[i] with
      ' ' -> s.[i] <- '_'
    | _ -> ()
  done;
  s

let string_of_values = fun values ->
  String.concat " " (List.map (fun (_, v) -> Pprz.string_of_value v) values)

let log = fun logging ac_name msg_name values ->
  match logging with
    Some log ->
      let s = string_of_values values in
      let t = U.gettimeofday () -. start_time in
      fprintf log "%.3f %s %s %s\n" t ac_name msg_name s; flush log
  | None -> ()


let log_and_parse = fun logging ac_name (a:Aircraft.aircraft) msg values ->
  log logging ac_name msg.Pprz.name values;
  let value = fun x -> try Pprz.assoc x values with Not_found -> failwith (sprintf "Error: field '%s' not found\n" x) in

  let fvalue = fun x -> 
    let f = fvalue (value x) in
      match classify_float f with
	FP_infinite | FP_nan ->
	  let msg = sprintf "Non normal number: %f in '%s %s %s'" f ac_name msg.Pprz.name (string_of_values values) in
	  Ground_Pprz.message_send my_id "TELEMETRY_ERROR" ["ac_id", Pprz.String ac_name;"message", Pprz.String (format_string_field msg)];
	  failwith msg
      | _ -> f
  and ivalue = fun x -> ivalue (value x) in
  if not (msg.Pprz.name = "DOWNLINK_STATUS") then
    a.last_bat_msg_date <- U.gettimeofday ();
  match msg.Pprz.name with
    "GPS" ->
      a.pos <- { utm_x = fvalue "utm_east" /. 100.;
		 utm_y = fvalue "utm_north" /. 100.;
		 utm_zone = ivalue "utm_zone" };
      a.unix_time <- LL.unix_time_of_tow (truncate (fvalue "itow" /. 1000.));
      a.itow <- Int32.of_float (fvalue "itow");
      (*Printf.fprintf stderr "itow %lu %ld\n" a.itow a.itow;*)
      a.gspeed  <- fvalue "speed" /. 100.;
      a.course  <- norm_course ((Deg>>Rad)(fvalue "course" /. 10.));
      a.agl     <- a.alt -. float (try Srtm.of_utm a.pos with _ -> 0);
      a.gps_mode <- check_index (ivalue "mode") gps_modes "GPS_MODE";
      if a.gspeed > 3. && a.ap_mode = _AUTO2 then
	Wind.update ac_name a.gspeed a.course
  | "GPS_SOL" ->
      a.gps_Pacc <- ivalue "Pacc"
  | "ESTIMATOR" ->
      a.alt     <- fvalue "z";
      a.climb   <- fvalue "z_dot"
  | "DESIRED" ->
      (* Trying to be compatible with old logs ... *)
      a.desired_east <- (try fvalue "x" with _ -> fvalue "desired_x");
      a.desired_north <- (try fvalue "y" with _ -> fvalue "desired_y");
      a.desired_altitude <- (try fvalue "altitude" with _ -> fvalue "desired_altitude");
      a.desired_climb <- (try fvalue "climb" with _ -> fvalue "desired_climb");
      begin try	a.desired_course <- norm_course (fvalue "course") with _ -> () end
  | "NAVIGATION_REF" ->
      a.nav_ref <- Some { utm_x = fvalue "utm_east"; utm_y = fvalue "utm_north"; utm_zone = ivalue "utm_zone" }
  | "ATTITUDE" ->
      a.roll <- (Deg>>Rad) (fvalue "phi");
      a.pitch <- (Deg>>Rad) (fvalue "theta")
  | "NAVIGATION" -> 
      a.cur_block <- ivalue "cur_block";
      a.cur_stage <- ivalue "cur_stage";
      a.dist_to_wp <- sqrt (fvalue "dist2_wp")
  | "BAT" ->
      a.throttle <- fvalue "throttle" /. 9600. *. 100.;
      a.kill_mode <- ivalue "kill_auto_throttle" <> 0;
      a.flight_time <- ivalue "flight_time";
      a.rpm <- a.throttle *. 100.;
      a.bat <- fvalue "voltage" /. 10.;
      a.stage_time <- ivalue "stage_time";
      a.block_time <- ivalue "block_time";
      a.energy <- ivalue "energy"
  | "FBW_STATUS" ->
      a.bat <- fvalue "vsupply" /. 10.		
  | "PPRZ_MODE" ->
      a.ap_mode <- check_index (ivalue "ap_mode") ap_modes "AP_MODE";
      a.gaz_mode <- check_index (ivalue "ap_gaz") gaz_modes "AP_GAZ";
      a.lateral_mode <- check_index (ivalue "ap_lateral") lat_modes "AP_LAT";
      a.horizontal_mode <- check_index (ivalue "ap_horizontal") horiz_modes "AP_HORIZ";
      let mcu1_status = ivalue "mcu1_status" in
      (** c.f. link_autopilot.h *)
      a.fbw.rc_status <- 
	if mcu1_status land 0b1 > 0
	then "OK"
	else if mcu1_status land 0b10 > 0
	then "NONE"
	else "LOST";
      a.fbw.rc_mode <-
	if mcu1_status land 0b1000 > 0
	then "FAILSAFE"
	else if mcu1_status land 0b100 > 0
	then "AUTO"
	else "MANUAL";
  | "CAM" ->
      a.cam.phi <- (Deg>>Rad) (fvalue  "phi");
      a.cam.theta <- (Deg>>Rad) (fvalue  "theta");
      a.cam.target <- (fvalue  "target_x", fvalue  "target_y")
  | "SVINFO" ->
      let i = ivalue "chn" in
      assert(i < Array.length a.svinfo);
      a.svinfo.(i) <- {  
	svid = ivalue "SVID";
	flags = ivalue "Flags";
	qi = ivalue "QI";
	cno = ivalue "CNO";
	elev = ivalue "Elev";
	azim = ivalue "Azim";
	age = 0
      }
  | "CIRCLE" ->
      begin
	match a.nav_ref, a.horizontal_mode with
	  Some nav_ref, 2 -> (** FIXME *)
	    a.horiz_mode <- Circle (LL.utm_add nav_ref (fvalue "center_east", fvalue "center_north"), ivalue "radius");
	    if !kml then kml_update_horiz_mode a
	| _ -> ()
      end
  | "SEGMENT" ->
      begin
	match a.nav_ref, a.horizontal_mode with
	  Some nav_ref, 1 -> (** FIXME *)
	    let p1 = LL.utm_add nav_ref (fvalue "segment_east_1", fvalue "segment_north_1")
	    and p2 = LL.utm_add nav_ref (fvalue "segment_east_2",  fvalue "segment_north_2") in
	    a.horiz_mode <- Segment (p1, p2);
	    if !kml then kml_update_horiz_mode a
	| _ -> ()
      end
  | "SURVEY" ->
      begin
	a.time_since_last_survey_msg <- 0.;
	match a.nav_ref with
	  Some nav_ref ->
	    let p1 = LL.utm_add nav_ref (fvalue "west", fvalue "south")
	    and p2 = LL.utm_add nav_ref (fvalue "east",  fvalue "north") in
	    a.survey <- Some (LL.of_utm WGS84 p1, LL.of_utm WGS84 p2)
	| None -> ()
      end
  | "CALIBRATION" ->
      a.throttle_accu <- fvalue "climb_sum_err"
  | "DL_VALUE" ->
      let i = ivalue "index" in
      if i < max_nb_dl_setting_values then begin
	a.dl_setting_values.(i) <- fvalue "value";
	a.nb_dl_setting_values <- max a.nb_dl_setting_values (i+1)
      end else
	failwith "Too much dl_setting values !!!"
  | "WP_MOVED" ->
      begin
	match a.nav_ref with
	  Some nav_ref ->
	    let utm_zone = try ivalue "utm_zone" with _ -> nav_ref.utm_zone in
	    let p = { LL.utm_x = fvalue "utm_east";
		      utm_y = fvalue "utm_north";
		      utm_zone = utm_zone } in
	    update_waypoint a (ivalue "wp_id") p (fvalue "alt")
	| None -> () (** Can't use this message  *)
      end
  | "FORMATION_SLOT_TM" ->
      Dl_Pprz.message_send "ground_dl" "FORMATION_SLOT" values
  | "FORMATION_STATUS_TM" ->
      Dl_Pprz.message_send "ground_dl" "FORMATION_STATUS" values
  | _ -> ()

(** Callback for a message from a registered A/C *)
let ac_msg = fun messages_xml log ac_name ac ->
  let module Tele_Pprz = Pprz.MessagesOfXml(struct let xml = messages_xml let name="telemetry" end) in
  fun m ->
    try
      let (msg_id, values) = Tele_Pprz.values_of_string m in
      let msg = Tele_Pprz.message_of_id msg_id in
      log_and_parse log ac_name ac msg values
    with
      Pprz.Unknown_msg_name (x, c) ->
	fprintf stderr "Unknown message %s in class %s from %s: %s\n%!" x c ac_name m
  | x -> prerr_endline (Printexc.to_string x)


(** If you are 1km above the ground, an angle of 89 degrees between the vertical and
your camera axis allow you to look 57km away: it should be enough ! **)
let cam_max_angle = (Deg>>Rad) 89.

let send_cam_status = fun a ->
  if a.gps_mode = gps_mode_3D then
    match a.nav_ref with
      None -> () (* No geo ref for camera target *)
    | Some nav_ref ->
	let h = a.agl in
	let phi_absolute = a.cam.phi -. a.roll
	and theta_absolute = a.cam.theta +. a.pitch in
	if phi_absolute > -. cam_max_angle && phi_absolute < cam_max_angle &&
	  theta_absolute > -. cam_max_angle && theta_absolute < cam_max_angle then
	  let dx = h *. tan phi_absolute
	  and dy = h *. tan theta_absolute in
	  let alpha = -. a.course in
	  let east = dx *. cos alpha -. dy *. sin alpha
	  and north = dx *. sin alpha +. dy *. cos alpha in
	  let utm = LL.utm_add a.pos (east, north) in
	  let wgs84 = LL.of_utm WGS84 utm in
	  let utm_target = LL.utm_add nav_ref a.cam.target in
	  let twgs84 =  LL.of_utm WGS84 utm_target in
	  let values = ["ac_id", Pprz.String a.id; 
			"cam_lat", Pprz.Float ((Rad>>Deg)wgs84.posn_lat);
			"cam_long", Pprz.Float ((Rad>>Deg)wgs84.posn_long); 
			"cam_target_lat", Pprz.Float ((Rad>>Deg)twgs84.posn_lat);
			"cam_target_long", Pprz.Float ((Rad>>Deg)twgs84.posn_long)] in
	  Ground_Pprz.message_send my_id "CAM_STATUS" values

let send_fbw = fun a ->
  let values = [ "ac_id", Pprz.String a.id;
		 "rc_mode", Pprz.String a.fbw.rc_mode;
		 "rc_status", Pprz.String a.fbw.rc_status ] in
  Ground_Pprz.message_send my_id "FLY_BY_WIRE"  values

let send_dl_values = fun a ->
  if a.nb_dl_setting_values > 0 then
    let csv = ref "" in
    for i = 0 to a.nb_dl_setting_values - 1 do
      csv := sprintf "%s%f," !csv a.dl_setting_values.(i)
    done;
    let vs = ["ac_id", Pprz.String a.id; "values", Pprz.String !csv] in
    Ground_Pprz.message_send my_id "DL_VALUES" vs

let send_svsinfo = fun a ->
  let svid = ref ""
  and flags= ref ""
  and qi = ref ""
  and cno = ref ""
  and elev = ref ""
  and azim = ref ""
  and age = ref "" in
  let concat = fun ref v ->
    ref := !ref ^ string_of_int v ^ "," in
  for i = 0 to gps_nb_channels - 1 do
    concat svid a.svinfo.(i).svid;
    concat flags a.svinfo.(i).flags;
    concat qi a.svinfo.(i).qi;
    concat cno a.svinfo.(i).cno;
    concat elev a.svinfo.(i).elev;
    concat azim a.svinfo.(i).azim;
    concat age a.svinfo.(i).age
  done;
  let f = fun s r -> (s, Pprz.String !r) in
  let vs = ["ac_id", Pprz.String a.id; 
	    "pacc", Pprz.Int a.gps_Pacc;
	    f "svid" svid; f "flags" flags; f "qi" qi;  f "msg_age" age; 
	    f "cno" cno; f "elev" elev; f "azim" azim] in
  Ground_Pprz.message_send my_id "SVSINFO" vs

let send_horiz_status = fun a ->
  match a.horiz_mode with
    Circle (utm, r) ->
      let wgs84 = LL.of_utm WGS84 utm in
      let vs = [ "ac_id", Pprz.String a.id; 
		 "circle_lat", Pprz.Float ((Rad>>Deg)wgs84.posn_lat);
		 "circle_long", Pprz.Float ((Rad>>Deg)wgs84.posn_long);
		 "radius", Pprz.Int r ] in
       Ground_Pprz.message_send my_id "CIRCLE_STATUS" vs
  | Segment (u1, u2) ->
      let geo1 = LL.of_utm WGS84 u1 in
      let geo2 = LL.of_utm WGS84 u2 in
      let vs = [ "ac_id", Pprz.String a.id; 
		 "segment1_lat", Pprz.Float ((Rad>>Deg)geo1.posn_lat);
		 "segment1_long", Pprz.Float ((Rad>>Deg)geo1.posn_long);
		 "segment2_lat", Pprz.Float ((Rad>>Deg)geo2.posn_lat);
		 "segment2_long", Pprz.Float ((Rad>>Deg)geo2.posn_long) ] in
      Ground_Pprz.message_send my_id "SEGMENT_STATUS" vs
  | UnknownHorizMode -> ()

let send_survey_status = fun a ->
  match a.survey with
    None -> 
      Ground_Pprz.message_send my_id "SURVEY_STATUS" ["ac_id", Pprz.String a.id]
  | Some (geo1, geo2) ->
      let vs = [ "ac_id", Pprz.String a.id; 
		 "south_lat", Pprz.Float ((Rad>>Deg)geo1.posn_lat);
		 "west_long", Pprz.Float ((Rad>>Deg)geo1.posn_long);
		 "north_lat", Pprz.Float ((Rad>>Deg)geo2.posn_lat);
		 "east_long", Pprz.Float ((Rad>>Deg)geo2.posn_long) ] in
      Ground_Pprz.message_send my_id "SURVEY_STATUS" vs
      


let send_wind = fun a ->
  let id = a.id in
  try
    let (wind_cap_rad, wind_polar, mean, stddev, _nb_sample) = Wind.get id in
    let vs =
      ["ac_id", Pprz.String id;
       "dir", Pprz.Float ((Rad>>Deg)wind_cap_rad);
       "wspeed", Pprz.Float wind_polar;
       "mean_aspeed", Pprz.Float mean;
       "stddev", Pprz.Float stddev] in
    Ground_Pprz.message_send my_id "WIND" vs
  with
    _exc -> ()

let send_telemetry_status = fun a ->
  let id = a.id in
  try
    let vs =
      ["ac_id", Pprz.String id;
       "time_since_last_bat_msg", Pprz.Float (U.gettimeofday () -. a.last_bat_msg_date)] in
    Ground_Pprz.message_send my_id "TELEMETRY_STATUS" vs
  with
    _exc -> ()

let send_moved_waypoints = fun a ->
  Hashtbl.iter
    (fun wp_id wp ->
      let geo = LL.of_utm WGS84 wp.wp_utm in
      let vs =
	["ac_id", Pprz.String a.id;
	 "wp_id", Pprz.Int wp_id;
	 "long", Pprz.Float ((Rad>>Deg)geo.posn_long);
	 "lat", Pprz.Float ((Rad>>Deg)geo.posn_lat);
	 "alt", Pprz.Float wp.altitude] in
      Ground_Pprz.message_send my_id "WAYPOINT_MOVED" vs)
    a.waypoints



let update_kml_ac = fun ac ->
  try
    let url_flight_plan = sprintf "http://%s:8889/var/%s/flight_plan.kml" !hostname ac.name in
    let blocks = ExtXml.child ac.flight_plan "blocks" in
    let block = ExtXml.child blocks (string_of_int ac.cur_block) in
    let block_name = ExtXml.attrib block "name" in
    let description = sprintf "<b><font color=\"green\">%s</font></b>: %s\nBat: <b><font color=\"red\">%.1fV</font></b>\nAGL: %.0fm\nSpeed: %.1fm/s" ap_modes.(ac.ap_mode) block_name ac.bat ac.agl ac.gspeed in
    let wgs84 = of_utm WGS84 ac.pos in
    let change = Kml.change_placemark ~description ac.name wgs84 ac.alt in
    let kml_changes = Kml.link_update url_flight_plan [change] in
    print_xml ac.name "ac_changes.kml" kml_changes
  with
    _ -> ()
	 

let send_aircraft_msg = fun ac ->
  try
    let a = Hashtbl.find aircrafts ac in
    let f = fun x -> Pprz.Float x in
    let wgs84 = try LL.of_utm WGS84 a.pos with _ -> LL.make_geo 0. 0. in
    let values = ["ac_id", Pprz.String ac;
		  "roll", f (Geometry_2d.rad2deg a.roll);
		  "pitch", f (Geometry_2d.rad2deg a.pitch);
		  "lat", f ((Rad>>Deg)wgs84.posn_lat);
		  "long", f ((Rad>>Deg) wgs84.posn_long);
		  "unix_time", f a.unix_time;
      "itow", Pprz.Int32 a.itow;
		  "speed", f a.gspeed;
		  "course", f (Geometry_2d.rad2deg a.course);
		  "alt", f a.alt;
		  "agl", f a.agl;
		  "climb", f a.climb] in
    Ground_Pprz.message_send my_id "FLIGHT_PARAM" values;

    (** send ACINFO messages if more than one A/C registered *)
    if Hashtbl.length aircrafts > 1 then
      begin
        let cm_of_m = fun f -> Pprz.Int (truncate (100. *. f)) in
        let ac_info = ["ac_id", Pprz.String ac;
          "utm_east", cm_of_m a.pos.utm_x;
          "utm_north", cm_of_m a.pos.utm_y;
          "course", Pprz.Int (truncate (10. *. (Geometry_2d.rad2deg a.course)));
          "alt", cm_of_m a.alt;
          "speed", cm_of_m a.gspeed;
          "climb", cm_of_m a.climb;
          "itow", Pprz.Int32 a.itow] in
        Dl_Pprz.message_send my_id "ACINFO" ac_info;
      end;

    if !kml then
      update_kml_ac a;

    begin
      match a.nav_ref with
	Some nav_ref ->
	  let target_utm = LL.utm_add nav_ref (a.desired_east, a.desired_north) in
	  let target_wgs84 = LL.of_utm WGS84 target_utm in 
	  let values = ["ac_id", Pprz.String ac; 
			"cur_block", Pprz.Int a.cur_block;
			"cur_stage", Pprz.Int a.cur_stage;
			"stage_time", Pprz.Int a.stage_time;
			"block_time", Pprz.Int a.block_time;
			"target_lat", f ((Rad>>Deg)target_wgs84.posn_lat);
			"target_long", f ((Rad>>Deg)target_wgs84.posn_long);
			"target_alt", Pprz.Float a.desired_altitude;
			"target_climb", Pprz.Float a.desired_climb;
			"target_course", Pprz.Float ((Rad>>Deg)a.desired_course);
			"dist_to_wp", Pprz.Float a.dist_to_wp
		      ] in
	  Ground_Pprz.message_send my_id "NAV_STATUS" values
      | None -> () (* No nav_ref yet *)
    end;

    let values = ["ac_id", Pprz.String ac; 
		  "throttle", f a.throttle;
		  "throttle_accu", f a.throttle_accu;
		  "rpm", f a.rpm;
		  "temp", f a.temp;
		  "bat", f a.bat;
		  "amp", f a.amp;
		  "energy", Pprz.Int a.energy] in
    Ground_Pprz.message_send my_id "ENGINE_STATUS" values;
    
    let ap_mode = get_indexed_value ap_modes a.ap_mode in
    let gaz_mode = get_indexed_value gaz_modes a.gaz_mode in
    let lat_mode = get_indexed_value lat_modes a.lateral_mode in
    let horiz_mode = get_indexed_value horiz_modes a.horizontal_mode in
    let gps_mode = get_indexed_value gps_modes a.gps_mode
    and kill_mode = if a.kill_mode then "ON" else "OFF" in
    let values = ["ac_id", Pprz.String ac; 
		  "flight_time", Pprz.Int a.flight_time;
		  "ap_mode", Pprz.String ap_mode; 
		  "gaz_mode", Pprz.String gaz_mode;
		  "lat_mode", Pprz.String lat_mode;
		  "horiz_mode", Pprz.String horiz_mode;
		  "gps_mode", Pprz.String gps_mode;
		  "kill_mode", Pprz.String kill_mode
		] in
    Ground_Pprz.message_send my_id "AP_STATUS" values;

    send_cam_status a;
    send_fbw a;
    send_svsinfo a;
    send_horiz_status a;
    
    a.time_since_last_survey_msg <- a.time_since_last_survey_msg +. float aircraft_msg_period /. 1000.;
    if a.time_since_last_survey_msg > 5. then (* FIXME Two missed messages *)
      a.survey <- None;

    send_survey_status a;
    send_dl_values a;
    send_moved_waypoints a;
    if !kml then
      kml_update_waypoints a;
    send_telemetry_status a
  with
    Not_found -> prerr_endline ac
  | x -> prerr_endline (Printexc.to_string x)

(** Check if it is a replayed A/C (c.f. sw/logalizer/play.ml) *)
let replayed = fun ac_id ->
  let n = String.length ac_id in
  if n > 6 && String.sub ac_id 0 6 = "replay" then
    (String.sub ac_id 6 (n - 6), "/var/replay/",  Xml.parse_file (Env.paparazzi_home // "var/replay/conf/conf.xml"))
  else
    (ac_id, "", conf_xml)

(* Store of unknown received A/C ids. To be able to report an error only once *)
let unknown_aircrafts = Hashtbl.create 5

let get_conf = fun real_id id conf_xml ->
  try 
    ExtXml.child conf_xml "aircraft" ~select:(fun x -> ExtXml.attrib x "ac_id" = id)
  with
    Not_found ->
      Hashtbl.add unknown_aircrafts real_id ();
      failwith (sprintf "Error: A/C '%s' not found" id)

      
let new_aircraft = fun real_id ->
  let id, root_dir, conf_xml = replayed real_id in
  let conf = get_conf real_id id conf_xml in
  let ac_name = ExtXml.attrib conf "name" in
  let fp_file = Env.paparazzi_home // root_dir // "var" // ac_name // "flight_plan.xml" in
  let xml_fp = ExtXml.child (Xml.parse_file fp_file) "flight_plan" in

  let airframe_file = Env.paparazzi_home // root_dir // "conf" // ExtXml.attrib conf "airframe" in
  let airframe_xml = Xml.parse_file airframe_file in
  
  let ac = Aircraft.new_aircraft real_id ac_name xml_fp airframe_xml in
  let update = fun () ->
    for i = 0 to Array.length ac.svinfo - 1 do
      ac.svinfo.(i).age <-  ac.svinfo.(i).age + 1;
    done in
  
  ignore (Glib.Timeout.add 1000 (fun _ -> update (); true));
  
  let messages_xml = Xml.parse_file (Env.paparazzi_home // root_dir // "conf" // "messages.xml") in
  ac, messages_xml
      
let check_alerts = fun a ->
  let bat_section = ExtXml.child a.airframe ~select:(fun x -> Xml.attrib x "name" = "BAT") "section" in
  let fvalue = fun name default ->
    try ExtXml.float_attrib (ExtXml.child bat_section ~select:(fun x -> ExtXml.attrib x "name" = name) "define") "value" with _ -> default in

  let catastrophic_level = fvalue "CATASTROPHIC_BAT_LEVEL" 9.
  and critic_level = fvalue "CRITIC_BAT_LEVEL" 10.
  and warning_level = fvalue "LOW_BAT_LEVEL" 10.5 in

  let send = fun level ->
    let vs = [ "ac_id", Pprz.String a.id; 
	       "level", Pprz.String level; 
	       "value", Pprz.Float a.bat] in
    Alerts_Pprz.message_send my_id "BAT_LOW" vs in
  if a.bat < catastrophic_level then send "CATASTROPHIC"
  else if a.bat < critic_level then send "CRITIC"
  else if a.bat < warning_level then send "WARNING"

let wind_clear = fun _sender vs ->
  Wind.clear (Pprz.string_assoc "ac_id" vs)

let periodic = fun period cb ->
  Glib.Timeout.add period (fun () -> cb (); true)


let register_periodic = fun ac x ->
  ac.periodic_callbacks <- x :: ac.periodic_callbacks

(** add the periodic airprox check for the aircraft (name) on all aircraft     *)
(**    already known                                                           *)
let periodic_airprox_check = fun name ->
  let thisac = Hashtbl.find aircrafts name in
  let aircrafts2list = fun () ->
    let list = ref [] in
    Hashtbl.iter (fun name _a -> list := name :: !list) aircrafts;
    !list in
  let ac_names = List.filter (fun a -> a <> name) (aircrafts2list ()) in
  let list_ac = List.map (fun name -> Hashtbl.find aircrafts name) ac_names in

  let check_airprox = fun ac ->
    try
      match Airprox.check_airprox thisac ac with
	None -> ()
      | Some level ->
	  let vs =
	    ["ac_id", Pprz.String (thisac.id ^ "," ^ ac.id) ; "level", Pprz.String level] in
	  Alerts_Pprz.message_send my_id "AIR_PROX" vs
    with
      x -> fprintf stderr "check_airprox: %s\n%!" (Printexc.to_string x)

 in
  
  List.iter 
    (fun ac ->
      register_periodic thisac (periodic aircraft_alerts_period (fun () -> check_airprox ac))
    ) list_ac


let register_aircraft = fun name a ->
  Hashtbl.add aircrafts name a;
  register_periodic a (periodic aircraft_msg_period (fun () -> send_aircraft_msg name));
  register_periodic a (periodic aircraft_alerts_period (fun () -> check_alerts a));
  periodic_airprox_check name;
  register_periodic a (periodic wind_msg_period (fun () -> send_wind a));
  Wind.new_ac name 36;
  ignore(Ground_Pprz.message_bind "WIND_CLEAR" wind_clear);

  if !kml then
    (* Build KML files *)
    let xml_fp = a.flight_plan in
    let kml_fp = Kml.flight_plan a.name xml_fp in
    print_xml a.name "flight_plan.kml" kml_fp;
    
    let url_flight_plan = sprintf "http://%s:8889/var/%s/flight_plan.kml" !hostname a.name in
    let url_ac_changes = sprintf "http://%s:8889/var/%s/ac_changes.kml" !hostname a.name in
    let url_wp_changes = sprintf "http://%s:8889/var/%s/wp_changes.kml" !hostname a.name in
    let url_route_changes = sprintf "http://%s:8889/var/%s/route_changes.kml" !hostname a.name in
    let kml_ac = Kml.aircraft a.name url_flight_plan [url_ac_changes; url_wp_changes; url_route_changes] in
    print_xml a.name "FollowMe.kml" kml_ac;;


(** Identifying message from an A/C *)
let ident_msg = fun log name ->
  try
    if not (Hashtbl.mem aircrafts name) && 
      not (Hashtbl.mem unknown_aircrafts name) then
      let ac, messages_xml = new_aircraft name in
      let ac_msg_closure = ac_msg messages_xml log name ac in
      let _b = Ivy.bind (fun _ args -> ac_msg_closure args.(0)) (sprintf "^%s +(.*)" name) in
      register_aircraft name ac;
      Ground_Pprz.message_send my_id "NEW_AIRCRAFT" ["ac_id", Pprz.String name]
  with
    exc -> prerr_endline (Printexc.to_string exc)
	
let new_color = fun () ->
  sprintf "#%02x%02x%02x" (Random.int 256) (Random.int 256) (Random.int 256)

(* Waits for new aircrafts *)
let listen_acs = fun log ->
  (** Wait for any message (they all are identified with the A/C) *)
  ignore (Ivy.bind (fun _ args -> ident_msg log args.(0)) "^(.*) ALIVE");
  ignore (Ivy.bind (fun _ args -> ident_msg log args.(0)) "^(.*) PPRZ_MODE")


let send_config = fun http _asker args ->
  let ac_id' = Pprz.string_assoc "ac_id" args in
  try
    let ac_id, root_dir, conf_xml = replayed ac_id' in

    let conf = ExtXml.child conf_xml "aircraft" ~select:(fun x -> ExtXml.attrib x "ac_id" = ac_id) in
    let ac_name = ExtXml.attrib conf "name" in
    let protocol =
      if http then
	sprintf "http://%s:8889" (Unix.gethostname ())
      else
	sprintf "file://%s" Env.paparazzi_home in
    let prefix = fun s -> sprintf "%s/%s%s" protocol root_dir s in

    (** Expanded flight plan and settings have been compiled in var/ *)
    let fp = prefix ("var" // ac_name // "flight_plan.xml")
    and af = prefix ("conf" // ExtXml.attrib conf "airframe")
    and rc = prefix ("conf" // ExtXml.attrib conf "radio")
    and settings = prefix  ("var" // ac_name // "settings.xml") in
    let col = try Xml.attrib conf "gui_color" with _ -> new_color () in
    let ac_name = try Xml.attrib conf "name" with _ -> "" in
    [ "ac_id", Pprz.String ac_id;
      "flight_plan", Pprz.String fp;
      "airframe", Pprz.String af;
      "radio", Pprz.String rc;
      "settings", Pprz.String settings;
      "default_gui_color", Pprz.String col;
      "ac_name", Pprz.String ac_name ]
  with
    Not_found ->
      failwith (sprintf "ground UNKNOWN %s" ac_id')     
    
let ivy_server = fun http ->
  ignore (Ground_Pprz.message_answerer my_id "AIRCRAFTS" send_aircrafts_msg);
  ignore (Ground_Pprz.message_answerer my_id "CONFIG" (send_config http))


let cm_of_m = fun f -> Pprz.Int (truncate (100. *. f))
let dl_id = "ground_dl" (* Hack, should be [my_id] *)

(** Got a ground.MOVE_WAYPOINT and send a datalink.MOVE_WP *)
let move_wp = fun logging _sender vs ->
  let f = fun a -> List.assoc a vs
  and ac_id = Pprz.string_assoc "ac_id" vs
  and deg7 = fun f -> Pprz.Int32 (Int32.of_float (Pprz.float_assoc f vs *. 1e7)) in
  let vs = [ "ac_id", Pprz.String ac_id;
	     "wp_id", f "wp_id";
	     "lat", deg7 "lat";
	     "lon", deg7 "long";
	     "alt", cm_of_m (Pprz.float_assoc "alt" vs) ] in
  Dl_Pprz.message_send dl_id "MOVE_WP" vs;
  log logging ac_id "MOVE_WP" vs

(** Got a DL_SETTING, and send an SETTING *)
let setting = fun logging _sender vs ->
  let ac_id = Pprz.string_assoc "ac_id" vs in
  let vs = ["ac_id", Pprz.String ac_id;
	    "index", List.assoc "index" vs; 
	    "value", List.assoc "value" vs] in
  Dl_Pprz.message_send dl_id "SETTING" vs;
  log logging ac_id "SETTING" vs


(** Got a JUMP_TO_BLOCK, and send an BLOCK *)
let jump_block = fun logging _sender vs ->
  let ac_id = Pprz.string_assoc "ac_id" vs in
  let vs = ["ac_id", Pprz.String ac_id; "block_id", List.assoc "block_id" vs] in
  Dl_Pprz.message_send dl_id "BLOCK" vs;
  log logging ac_id "BLOCK" vs

(** Got a RAW_DATALINK,send its contents *)
let raw_datalink = fun logging _sender vs ->
  let ac_id = Pprz.string_assoc "ac_id" vs
  and m = Pprz.string_assoc "message" vs in
  for i = 0 to String.length m - 1 do
    if m.[i] = ';' then m.[i] <- ' '
  done;
  let msg_id, vs = Dl_Pprz.values_of_string m in
  let msg = Dl_Pprz.message_of_id msg_id in
  Dl_Pprz.message_send dl_id msg.Pprz.name vs;
  log logging ac_id msg.Pprz.name vs

(** Get the 'ground' uplink messages, log them and send 'datalink' messages *)
let ground_to_uplink = fun logging ->
  let bind_log_and_send = fun name handler ->
    ignore (Ground_Pprz.message_bind name (handler logging)) in
  bind_log_and_send "MOVE_WAYPOINT" move_wp;
  bind_log_and_send "DL_SETTING" setting;
  bind_log_and_send "JUMP_TO_BLOCK" jump_block;
  bind_log_and_send "RAW_DATALINK" raw_datalink


(* main loop *)
let _ =
  let ivy_bus = ref "127.255.255.255:2010"
  and logging = ref true
  and http = ref false in

  let options =
    [ "-b", Arg.String (fun x -> ivy_bus := x), (sprintf "Bus\tDefault is %s" !ivy_bus);
      "-n", Arg.Clear logging, "Disable log";
      "-kml", Arg.Set kml, "Enable KML file updating";
      "-hostname", Arg.Set_string hostname, "<hostname> Set the address for the http server";
      "-http", Arg.Set http, "Send http: URLs (default is file:)"] in
  Arg.parse (options)
    (fun x -> Printf.fprintf stderr "%s: Warning: Don't do anything with '%s' argument\n" Sys.argv.(0) x)
    "Usage: ";

  Srtm.add_path srtm_path;

  Ivy.init "Paparazzi server" "READY" (fun _ _ -> ());
  Ivy.start !ivy_bus;

  let logging = 
    if !logging then
      (* Opens the log file *)
      let log = logger () in
      (* Waits for new simulated aircrafts *)
      (Some log)
    else
      None in
  listen_acs logging;

  ground_to_uplink logging;

  (* Waits for client requests on the Ivy bus *)
  ivy_server !http;
  
  let loop = Glib.Main.create true in
  while Glib.Main.is_running loop do ignore (Glib.Main.iteration true) done
