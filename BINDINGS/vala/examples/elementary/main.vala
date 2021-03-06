/**
 * Copyright (C) 2009 Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

Elm.Win win;
unowned Elm.Box? box;

public void add_test( T.Abstract t )
{
    unowned Elm.Button? bt = Elm.Button.add( win );
    bt.text_set( t.name() );
	bt.smart_callback_add( "clicked", t.run );
    bt.show();
    bt.size_hint_align_set( 0.5, 0.5 );
    bt.size_hint_weight_set( 1.0, 1.0 );
    box.pack_end( bt );
}

public int main( string[] args )
{
    debug( "main()" );
    Elm.init( args );

    win = new Elm.Win( null, "myWindow", Elm.WinType.BASIC );
    win.title_set( "Elementary meets Vala" );
    win.autodel_set( true );
    win.resize( 320, 320 );
    win.smart_callback_add( "delete,request", Elm.exit );

/*
    unowned Elm.Bg? bg = Elm.Bg.add( win );
    bg.size_hint_weight_set( 1.0, 1.0 );
    bg.show();
    win.resize_object_add( bg );
*/

    unowned Elm.Layout? layout = Elm.Layout.add( win );
    layout.file_set( "/usr/local/share/elementary/objects/test.edj", "layout" );
    layout.size_hint_weight_set( 1.0, 1.0 );
    layout.show();
    win.resize_object_add( layout );

    box = Elm.Box.add( win );
    box.size_hint_weight_set( 1.0, 1.0 );
    win.resize_object_add( box );

    var test1 = new T.Background();
    add_test( test1 );
    var test2 = new T.Hoversel();
    add_test( test2 );
    var test3 = new T.Genlist();
    add_test( test3 );

    box.homogeneous_set( true );

    box.show();
    win.show();
    Elm.run();
    Elm.shutdown();
    return 0;
}
