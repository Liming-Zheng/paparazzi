open Printf

module Tm_Pprz = Pprz.Messages (struct let name = "telemetry" end)

let ac_id = ref "1"
let nav_ref = ref (Latlong.utm_of Latlong.WGS84 (Latlong.make_geo_deg 43.46223 1.27289)) (* Muret *)

let gps_status = ref 0

let get_status = fun _ values ->
  let ivalue = fun x -> try Pprz.int_assoc x values with Not_found ->
    failwith (sprintf "Error: field '%s' not found\n" x) in

  let ap_mode = ivalue "ap_mode" in
  let pprz_mode = ref 0 in
  if ap_mode = 0 || ap_mode = 1 || ap_mode = 2 || ap_mode = 4 then pprz_mode := 0
  else if ap_mode = 3 || ap_mode = 5 then pprz_mode := 1
  else if ap_mode = 6 || ap_mode = 7 || ap_mode = 8 then pprz_mode := 2;
  let rc_status = ivalue "rc_status" in
  let mcu1_status = ref 2 in
  if rc_status = 0 then mcu1_status := 1
  else if rc_status = 1 then mcu1_status := 2
  else if rc_status = 2 then mcu1_status := 0;
  gps_status := (ivalue "gps_status");
  let bat = ivalue "vsupply" in
  let mode_values = [
    "ap_mode",       Pprz.Int !pprz_mode;
    "ap_gaz",        Pprz.Int 0;
    "ap_lateral",    Pprz.Int 0;
    "ap_horizontal", Pprz.Int 0;
    "if_calib_mode", Pprz.Int 0;
    "mcu1_status",   Pprz.Int !mcu1_status] in
  Tm_Pprz.message_send !ac_id "PPRZ_MODE" mode_values;
  let bat_values = [
    "rc_status",  Pprz.Int 0;
    "mode",       Pprz.Int 0;
    "vsupply",    Pprz.Int bat] in
  Tm_Pprz.message_send !ac_id "FBW_STATUS" bat_values

let get_fp = fun _ values ->
  let i32value = fun x -> try Pprz.int32_assoc x values with Not_found ->
    failwith (sprintf "Error: field '%s' not found\n" x) in

  let lat    = Int32.to_float (i32value "lat") /. 1e7
  and lon    = Int32.to_float (i32value "lon") /. 1e7
  and alt    = Int32.to_float (i32value "alt") /. 100.
  and vnorth = Int32.to_float (i32value "vnorth")
  and veast  = Int32.to_float (i32value "veast")
  and phi    = i32value "phi"
  and theta  = i32value "theta"
  and psi    = i32value "psi"
  and carrot_lat = Int32.to_float (i32value "carrot_lat") /. 1e7
  and carrot_lon = Int32.to_float (i32value "carrot_lon") /. 1e7 in
  let utm = Latlong.utm_of Latlong.WGS84 (Latlong.make_geo_deg lat lon) in
  let carrot_utm = Latlong.utm_of Latlong.WGS84 (Latlong.make_geo_deg carrot_lat carrot_lon) in
  let gspeed = sqrt(vnorth*.vnorth +. veast*.veast) in
  let power_12 = 1 lsl 12 in

  let gps_values = [
    "mode",       Pprz.Int !gps_status;
    "utm_east",   Pprz.Int32 (Int32.of_float (utm.Latlong.utm_x *. 100.));
    "utm_north",  Pprz.Int32 (Int32.of_float (utm.Latlong.utm_y *. 100.));
    "course",     Pprz.Int (573 * (Int32.to_int psi) / power_12);
    "alt",        Pprz.Int 0;
    "speed",      Pprz.Int (int_of_float gspeed);
    "climb",      Pprz.Int 0;
    "itow",       Pprz.Int32 (Int32.of_int 0);
    "utm_zone",   Pprz.Int utm.Latlong.utm_zone;
    "gps_nb_err", Pprz.Int 0] in
  Tm_Pprz.message_send !ac_id "GPS" gps_values;

  let est_values = [
    "z", Pprz.Float alt;
    "z_dot", Pprz.Float 0.] in
  Tm_Pprz.message_send !ac_id "ESTIMATOR" est_values;

  let att_values = [
    "phi",   Pprz.Int (truncate (57.3 *. Int32.to_float(phi)   /. float_of_int(power_12)));
    "psi",   Pprz.Int (truncate (57.3 *. Int32.to_float(psi)   /. float_of_int(power_12)));
    "theta", Pprz.Int (truncate (57.3 *. Int32.to_float(theta) /. float_of_int(power_12)))] in
  Tm_Pprz.message_send !ac_id "ATTITUDE" att_values;

  (*
  let dx = (carrot_utm.Latlong.utm_x -. !nav_ref.Latlong.utm_x) in
  let dy = (carrot_utm.Latlong.utm_y -. !nav_ref.Latlong.utm_y) in *)
  let (dx, dy) = Latlong.utm_sub carrot_utm !nav_ref in
  let desired_val = [
    "roll",     Pprz.Float 0.;
    "pitch",    Pprz.Float 0.;
    "course",   Pprz.Float 0.;
    "x",        Pprz.Float dx;
    "y",        Pprz.Float dy;
    "altitude", Pprz.Float 0.;
    "climb",    Pprz.Float 0.] in
  Tm_Pprz.message_send !ac_id "DESIRED" desired_val

let get_nav_ref = fun _ values ->
  let i32value = fun x -> try Pprz.int32_assoc x values with Not_found ->
    failwith (sprintf "Error: field '%s' not found\n" x) in

  let lat = Int32.to_float (i32value "lat") /. 1e7
  and lon = Int32.to_float (i32value "lon") /. 1e7 in
  nav_ref := Latlong.utm_of Latlong.WGS84 (Latlong.make_geo_deg lat lon);

  let nav_ref_val = [
    "utm_east",  Pprz.Int32 (Int32.of_float !nav_ref.Latlong.utm_x);
    "utm_north", Pprz.Int32 (Int32.of_float !nav_ref.Latlong.utm_y);
    "utm_zone",  Pprz.Int   !nav_ref.Latlong.utm_zone] in
  Tm_Pprz.message_send !ac_id "NAVIGATION_REF" nav_ref_val

let get_wp_moved = fun _ values ->
  let i32value = fun x -> try Pprz.int32_assoc x values with Not_found ->
    failwith (sprintf "Error: field '%s' not found\n" x) in

  let wp_id = Pprz.int_assoc "wp_id" values in
  let lat = Int32.to_float (i32value "lat") /. 1e7
  and lon = Int32.to_float (i32value "lon") /. 1e7
  and alt = Int32.to_float (i32value "alt") *. 100. in
  let wp = Latlong.utm_of Latlong.WGS84 (Latlong.make_geo_deg lat lon) in

  let wp_moved_val = [
    "wp_id",     Pprz.Int wp_id;
    "utm_east",  Pprz.Float wp.Latlong.utm_x;
    "utm_north", Pprz.Float wp.Latlong.utm_y;
    "alt",       Pprz.Float alt;
    "utm_zone",  Pprz.Int   wp.Latlong.utm_zone] in
  Tm_Pprz.message_send !ac_id "WP_MOVED" wp_moved_val



(*********************** Main ************************************************)
let _ =
  let ivy_bus = ref "127.255.255.255:2010" in

  Arg.parse
    [ "-b", Arg.String (fun x -> ivy_bus := x), "Bus\tDefault is 127.255.255.255:2010";
      "-a", Arg.String (fun x -> ac_id := x), "AC id\tDefault is 1"]
    (fun x -> prerr_endline ("WARNING: don't do anything with "^x))
    "Usage: ";

  (** Connect to the Ivy bus *)
  Ivy.init "Paparazzi BOOZ translator" "READY" (fun _ _ -> ());
  Ivy.start !ivy_bus;


  ignore (Tm_Pprz.message_bind "BOOZ_STATUS" get_status);
  ignore (Tm_Pprz.message_bind "BOOZ2_FP" get_fp);
  ignore (Tm_Pprz.message_bind "BOOZ2_NAV_REF" get_nav_ref);
  ignore (Tm_Pprz.message_bind "WP_MOVED_LLA" get_wp_moved);

  let loop = Glib.Main.create true in
  while Glib.Main.is_running loop do ignore (Glib.Main.iteration true) done
