(*
 * $Id$
 *
 * Paparazzi widget renderers
 *  
 * Copyright (C) 2008 ENAC
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

open Printf
module PC = Papget_common
let (//) = Filename.concat

class type movable_item =
    object
      inherit GnoCanvas.base_item
      method set : GnomeCanvas.group_p list -> unit
    end

class type t =
  object
    method tag : string
    method edit : (GObj.widget -> unit) -> unit
    method item : movable_item
    method update : string -> unit
    method config : unit -> Xml.xml list
  end


(*************************** Text ***********************************)
class canvas_text = fun ?(config=[]) canvas_group x y ->
  let group = GnoCanvas.group ~x ~y canvas_group in
  let text = GnoCanvas.text ~text:"_" group in
  object (self)
    val mutable format = PC.get_prop "format" config "%.2f"
    val mutable size = float_of_string (PC.get_prop "size" config "15.")
    val mutable color = PC.get_prop "color" config "green"
 
    method tag = "Text"
    method item = (group :> movable_item)
    method config = fun () ->
      [ PC.property "format" format;
	PC.float_property "size" size;
	PC.property "color" color ]
    method update = fun (value : string) ->
      let renderer = fun x -> sprintf (Obj.magic format) (float_of_string x) in
      text#set [`SIZE_POINTS size; `TEXT (renderer value); `FILL_COLOR color; `ANCHOR `NW]


    method edit = fun (pack:GObj.widget -> unit) ->
      let file = Env.paparazzi_src // "sw" // "lib" // "ocaml" // "widgets.glade" in
      let text_editor = new Gtk_papget_text_editor.table_text_editor ~file () in
      pack text_editor#table_text_editor#coerce;

      (* Initialize the entries *)
      text_editor#entry_format#set_text format;
      text_editor#spinbutton_size#set_value size;
      
      (* Connect the entries *)
      let callback = fun () ->
	format <- text_editor#entry_format#text in
      ignore (text_editor#entry_format#connect#activate ~callback);
      let callback = fun () ->
	size <- text_editor#spinbutton_size#value in
      ignore (text_editor#spinbutton_size#connect#value_changed ~callback);
  end


(***************************Vertical Ruler ***********************************)
let affine_pos_and_angle xw yw angle =
  let cos_a = cos angle in
  let sin_a = sin angle in
  [| cos_a ; sin_a ; ~-. sin_a; cos_a; xw ; yw |]
let affine_pos xw yw = affine_pos_and_angle xw yw 0.


class canvas_ruler = fun ?(config=[]) canvas_group x y ->
  let h = float_of_string (PC.get_prop "height" config "100.")
  and index_on_right = bool_of_string (PC.get_prop "index_on_right" config "false")
  and scale = float_of_string (PC.get_prop "scale" config "2.")
  and w = float_of_string (PC.get_prop "width" config "32.")
  and step = int_of_string (PC.get_prop "step" config "10") in
  let text_props=[`ANCHOR `CENTER; `FILL_COLOR "white"]
  and index_width = 10. in

  let root = GnoCanvas.group ~x ~y canvas_group in
  let r = GnoCanvas.group root in
  
  let props = (text_props@[`ANCHOR `EAST]) in
  
  (* One step drawer *)
  let draw = fun i value ->
    let i = i * step in
    let y = -. scale *. (float i -. value) in
    if y >= -. h && y <= h then begin
      let text = Printf.sprintf "%d" i in
      ignore (GnoCanvas.text ~text ~props ~y ~x:(w*.0.75) r);
      ignore(GnoCanvas.line ~points:[|w*.0.8;y;w-.1.;y|] ~fill_color:"white" r)
    end;
    let y = y -. float step /. 2. *. scale in
    if y >= -. h && y <= h then
      ignore(GnoCanvas.line ~points:[|w*.0.8;y;w-.1.;y|] ~fill_color:"white" r)
  in
  
  let drawer = fun value ->
    (* Remove previous items *)
    List.iter (fun i -> i#destroy ()) r#get_items;
    let v = truncate value / step in
    let k = truncate (h /. scale) / step in
    for i = Pervasives.max 0 (v - k) to (v + k) do
      draw i value
    done in
  
  (** Yellow index *)
  let _ = GnoCanvas.line ~points:[|0.;0.;w-.1.;0.|] ~fill_color:"yellow" root in
  let s = index_width in
  let idx = GnoCanvas.polygon ~points:[|0.;0.;-.s;s/.2.;-.s;-.s/.2.|] ~fill_color:"yellow" root in
  let () = 
    if index_on_right then
      idx#affine_absolute (affine_pos_and_angle w 0. Latlong.pi) in
  
  object
    method tag = "Ruler"
    method edit = fun (pack:GObj.widget -> unit) -> ()
    method update = fun value ->
      let value = float_of_string value in
      drawer value
    method item = (root :> movable_item)
    method config = fun () -> config (* Not editable *)
  end

(*************************** Gauge ***********************************)
class canvas_gauge = fun ?(config=[]) canvas_group x y ->
  let size = PC.get_prop "size" config "50." in
  let text = PC.get_prop "text" config "" in
  (*let text_props = [`ANCHOR `CENTER; `FILL_COLOR "white"] in*)

  let r1 = Pervasives.max 10. ((float_of_string size) /. 2.) in
  let r2 = r1 +. 3. in
  let r3 = 3.5 in
  let max_rot = 2. *. Latlong.pi /. 3. in

  let root = GnoCanvas.group ~x ~y canvas_group in
  (*let gauge = GnoCanvas.group root in*)
  
  (*let props = (text_props@[`ANCHOR `EAST]) in*)

  let _ = GnoCanvas.ellipse ~x1:r2 ~y1:r2 ~x2:(-.r2) ~y2:(-.r2)
    ~props:[`NO_FILL_COLOR; `OUTLINE_COLOR "grey"; `WIDTH_UNITS 6.]  root in
  let points = [|0.;-.r1;0.;-.r1+.3.|] in
  let props = [`WIDTH_UNITS 2.; `FILL_COLOR "red"] in
  let _ = GnoCanvas.line ~points ~props root in
  let il = GnoCanvas.line ~points ~props root in
  let () = il#affine_absolute (affine_pos_and_angle 0. 0. (-. Latlong.pi /. 3.)) in
  let ill = GnoCanvas.line ~points ~props root in
  let () = ill#affine_absolute (affine_pos_and_angle 0. 0. (-. 2. *. Latlong.pi /. 3.)) in
  let ir = GnoCanvas.line ~points ~props root in
  let () = ir#affine_absolute (affine_pos_and_angle 0. 0. (Latlong.pi /. 3.)) in
  let irr = GnoCanvas.line ~points ~props root in
  let () = irr#affine_absolute (affine_pos_and_angle 0. 0. (2. *. Latlong.pi /. 3.)) in

  let idx = GnoCanvas.polygon ~points:[|r3-.0.2;0.;0.;-.r1;-.(r3-.0.2);0.|] 
    ~props:[`FILL_COLOR "red"; `OUTLINE_COLOR "white"] root in
  let _ = GnoCanvas.ellipse ~x1:r3 ~y1:r3 ~x2:(-.r3) ~y2:(-.r3) ~props:[`OUTLINE_COLOR "grey"] ~fill_color:"red" root in
  let text_min = GnoCanvas.text ~x:(-.r1) ~y:(r1/.2.) ~props:[`ANCHOR `NE; `FILL_COLOR "green"] root in
  let text_max = GnoCanvas.text ~x:r1 ~y:(r1/.2.) ~props:[`ANCHOR `NW; `FILL_COLOR "green"] root in
  let text_mid = GnoCanvas.text ~x:0. ~y:(-.r2-.3.) ~props:[`ANCHOR `SOUTH; `FILL_COLOR "green"] root in
  let text_text = GnoCanvas.text ~x:0. ~y:(r2+.3.) ~props:[`ANCHOR `NORTH; `FILL_COLOR "green"] root in

  object
    val mutable min = PC.get_prop "min" config "-50."
    val mutable max = PC.get_prop "max" config "50."
    val mutable text = PC.get_prop "text" config ""

    method tag = "Gauge"
    method edit = fun (pack:GObj.widget -> unit) ->
      let file = Env.paparazzi_src // "sw" // "lib" // "ocaml" // "widgets.glade" in
      let gauge_editor = new Gtk_papget_gauge_editor.table_gauge_editor ~file () in
      pack gauge_editor#table_gauge_editor#coerce;

      (* Initialize the entries *)
      gauge_editor#entry_min#set_text min;
      gauge_editor#entry_max#set_text max;
      gauge_editor#entry_text#set_text text;

      (* Connect the entries *)
      let callback = fun () ->
        min <- gauge_editor#entry_min#text in
      ignore (gauge_editor#entry_min#connect#activate ~callback);
      let callback = fun () ->
        max <- gauge_editor#entry_max#text in
      ignore (gauge_editor#entry_max#connect#activate ~callback);
      let callback = fun () ->
        text <- gauge_editor#entry_text#text in
      ignore (gauge_editor#entry_text#connect#activate ~callback);

    method update = fun value ->
      let value = float_of_string value in
      (* Gauge drawer *)
      let fmin = float_of_string min in
      let fmax = float_of_string max in
      let rot = ref (-.max_rot +. 2. *. max_rot *. (value -. fmin) /. (fmax -. fmin)) in
      if !rot > max_rot then rot := max_rot;
      if !rot < -.max_rot then rot := -.max_rot;
      idx#affine_absolute (affine_pos_and_angle 0. 0. !rot);
      text_min#set [`TEXT min];
      text_max#set [`TEXT max];
      text_mid#set [`TEXT (string_of_float ((fmin +. fmax)/.2.))];
      text_text#set [`TEXT text]

    method item = (root :> movable_item)
    method config = fun () ->
      [ PC.property "min" min;
        PC.property "max" max;
        PC.property "size" size;
        PC.property "text" text ]
  end

(****************************************************************************)
class canvas_button = fun ?(config=[]) canvas_group x y ->
  let icon = PC.get_prop "icon" config "icon_file" in
  let pixbuf = GdkPixbuf.from_file (Env.gcs_icons_path // icon) in
  let group = GnoCanvas.group ~x ~y canvas_group in
  let _item = GnoCanvas.pixbuf ~pixbuf group in
  object
    method tag = "Button"
    method item = (group :> movable_item)
    method edit = fun (pack:GObj.widget -> unit) -> ()
    method update = fun (value:string) -> ()
    method config = fun () ->
      [ PC.property "icon" icon]
  end


let renderers =
  [ (new canvas_text :> ?config:Xml.xml list -> #GnoCanvas.group -> float -> float -> t);
    (new canvas_ruler :> ?config:Xml.xml list -> #GnoCanvas.group -> float -> float -> t);
    (new canvas_gauge :> ?config:Xml.xml list -> #GnoCanvas.group -> float -> float -> t) ] 

let lazy_tagged_renderers = lazy
    (let x = 0. and y = 0.
    and group = (GnoCanvas.canvas ())#root in
    List.map
      (fun constructor ->
	let o = constructor ?config:None group x y in
	(o#tag, constructor))
      renderers)

