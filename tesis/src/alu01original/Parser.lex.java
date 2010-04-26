import java.lang.System;
import java_cup.runtime.*;
import java.awt.Point;
class Utility {
	private static final String mensaje[] = {
		"No se cerro correctamente un comentario.",
		"No se cerro correctamente un string.",
		"Caracter Ilegal.",
		"Error con el hint definido"
		};
	public static final int E_COMENT = 0; 
	public static final int E_STR = 1; 
	public static final int E_CARACTERI = 2; 
	public static final int E_HINT = 3;
	public static void error(int code, String valor, Point p)
	{
	   System.out.println("Error en (" + (p.x+1) + "," + p.y + "): " +  mensaje[code] + valor);
	   System.out.println("El migrador no puede ser generado");
	   System.exit(1);
	}
}


class Yylex implements java_cup.runtime.Scanner {
	private final int YY_BUFFER_SIZE = 592;
	private final int YY_F = -1;
	private final int YY_NO_STATE = -1;
	private final int YY_NOT_ACCEPT = 0;
	private final int YY_START = 1;
	private final int YY_END = 2;
	private final int YY_NO_ANCHOR = 4;
	private final int YY_BOL = 65536;
	private final int YY_EOF = 65537;

	private boolean comentarioAbierto = false;
	private boolean hintAbierto = false;
	private int lineaComentario;
	private int colComentario;
	private int lineaHint;
	private int colHint;
	public int obtYyline(){
	    return yyline;
	}
	public int obtYychar(){
	    return yychar;
	}
	public String obtYytext(){
	    String linea = new java.lang.String(yy_buffer,yy_buffer_start,yy_buffer.length-yy_buffer_index);
	    StringBuffer sblinea = new java.lang.StringBuffer().append(yy_buffer);
	    //String linea = new java.lang.String(sblinea);
	    int indice = linea.indexOf('\n');
	    String[] lineas = new String(yy_buffer).split("\n");
	    for(int i =0;i< lineas.length;i++){
		if((lineas[i].trim()).equals((linea.substring(0,linea.indexOf('\n')+1)).trim())){
		    if(i<1){
			yyline++;
			return lineas[i];
		    }
		    else{	
			if(lineas[i-1].trim().length() == 0){ 
			    int j=1;
			    while((i-j)>0){
				if(lineas[i-j].trim().length() == 0)
				    j++;
				else{
				    yyline=(yyline+1)-j;
				    return lineas[i-j]; 
				}
			    }
			}
			else{
			    return lineas[i-1];
			}
		    }
		}
	    }
	    if(indice != -1){
		return (linea.substring(0,linea.indexOf('\n')+1));
	    }
	    else{
		linea = new java.lang.String(yy_buffer,(yy_buffer_start-yychar+1),yy_buffer.length-yy_buffer_index);
	    }
	    return linea;
	}	
	public Symbol obtnext_token(){
	    try{return next_token();}
	    catch(Exception e){return new Symbol(0,new Integer(0));}}
	private java.io.BufferedReader yy_reader;
	private int yy_buffer_index;
	private int yy_buffer_read;
	private int yy_buffer_start;
	private int yy_buffer_end;
	private char yy_buffer[];
	private int yychar;
	private int yyline;
	private boolean yy_at_bol;
	private int yy_lexical_state;

	Yylex (java.io.Reader reader) {
		this ();
		if (null == reader) {
			throw (new Error("Error: Bad input stream initializer."));
		}
		yy_reader = new java.io.BufferedReader(reader);
	}

	Yylex (java.io.InputStream instream) {
		this ();
		if (null == instream) {
			throw (new Error("Error: Bad input stream initializer."));
		}
		yy_reader = new java.io.BufferedReader(new java.io.InputStreamReader(instream));
	}

	private Yylex () {
		yy_buffer = new char[YY_BUFFER_SIZE];
		yy_buffer_read = 0;
		yy_buffer_index = 0;
		yy_buffer_start = 0;
		yy_buffer_end = 0;
		yychar = 0;
		yyline = 0;
		yy_at_bol = true;
		yy_lexical_state = YYINITIAL;

	yychar=1;
	}

	private boolean yy_eof_done = false;
	private void yy_do_eof () {
		if (false == yy_eof_done) {

	if(comentarioAbierto)
		Utility.error(Utility.E_COMENT,"",new Point(lineaComentario,colComentario));
		}
		yy_eof_done = true;
	}
	private final int YYINITIAL = 0;
	private final int HINTS = 1;
	private final int COMENTARIO2 = 3;
	private final int COMENTARIO1 = 2;
	private final int yy_state_dtrans[] = {
		0,
		22,
		26,
		30
	};
	private void yybegin (int state) {
		yy_lexical_state = state;
	}
	private int yy_advance ()
		throws java.io.IOException {
		int next_read;
		int i;
		int j;

		if (yy_buffer_index < yy_buffer_read) {
			return yy_buffer[yy_buffer_index++];
		}

		if (0 != yy_buffer_start) {
			i = yy_buffer_start;
			j = 0;
			while (i < yy_buffer_read) {
				yy_buffer[j] = yy_buffer[i];
				++i;
				++j;
			}
			yy_buffer_end = yy_buffer_end - yy_buffer_start;
			yy_buffer_start = 0;
			yy_buffer_read = j;
			yy_buffer_index = j;
			next_read = yy_reader.read(yy_buffer,
					yy_buffer_read,
					yy_buffer.length - yy_buffer_read);
			if (-1 == next_read) {
				return YY_EOF;
			}
			yy_buffer_read = yy_buffer_read + next_read;
		}

		while (yy_buffer_index >= yy_buffer_read) {
			if (yy_buffer_index >= yy_buffer.length) {
				yy_buffer = yy_double(yy_buffer);
			}
			next_read = yy_reader.read(yy_buffer,
					yy_buffer_read,
					yy_buffer.length - yy_buffer_read);
			if (-1 == next_read) {
				return YY_EOF;
			}
			yy_buffer_read = yy_buffer_read + next_read;
		}
		return yy_buffer[yy_buffer_index++];
	}
	private void yy_move_end () {
		if (yy_buffer_end > yy_buffer_start &&
		    '\n' == yy_buffer[yy_buffer_end-1])
			yy_buffer_end--;
		if (yy_buffer_end > yy_buffer_start &&
		    '\r' == yy_buffer[yy_buffer_end-1])
			yy_buffer_end--;
	}
	private boolean yy_last_was_cr=false;
	private void yy_mark_start () {
		int i;
		for (i = yy_buffer_start; i < yy_buffer_index; ++i) {
			if ('\n' == yy_buffer[i] && !yy_last_was_cr) {
				++yyline;
			}
			if ('\r' == yy_buffer[i]) {
				++yyline;
				yy_last_was_cr=true;
			} else yy_last_was_cr=false;
		}
		yychar = yychar
			+ yy_buffer_index - yy_buffer_start;
		yy_buffer_start = yy_buffer_index;
	}
	private void yy_mark_end () {
		yy_buffer_end = yy_buffer_index;
	}
	private void yy_to_mark () {
		yy_buffer_index = yy_buffer_end;
		yy_at_bol = (yy_buffer_end > yy_buffer_start) &&
		            ('\r' == yy_buffer[yy_buffer_end-1] ||
		             '\n' == yy_buffer[yy_buffer_end-1] ||
		             2028/*LS*/ == yy_buffer[yy_buffer_end-1] ||
		             2029/*PS*/ == yy_buffer[yy_buffer_end-1]);
	}
	private java.lang.String yytext () {
		return (new java.lang.String(yy_buffer,
			yy_buffer_start,
			yy_buffer_end - yy_buffer_start));
	}
	private int yylength () {
		return yy_buffer_end - yy_buffer_start;
	}
	private char[] yy_double (char buf[]) {
		int i;
		char newbuf[];
		newbuf = new char[2*buf.length];
		for (i = 0; i < buf.length; ++i) {
			newbuf[i] = buf[i];
		}
		return newbuf;
	}
	private final int YY_E_INTERNAL = 0;
	private final int YY_E_MATCH = 1;
	private java.lang.String yy_error_string[] = {
		"Error: Internal error.\n",
		"Error: Unmatched input.\n"
	};
	private void yy_error (int code,boolean fatal) {
		java.lang.System.out.print(yy_error_string[code]);
		java.lang.System.out.flush();
		if (fatal) {
			throw new Error("Fatal Error.\n");
		}
	}
	private int[][] unpackFromString(int size1, int size2, String st) {
		int colonIndex = -1;
		String lengthString;
		int sequenceLength = 0;
		int sequenceInteger = 0;

		int commaIndex;
		String workString;

		int res[][] = new int[size1][size2];
		for (int i= 0; i < size1; i++) {
			for (int j= 0; j < size2; j++) {
				if (sequenceLength != 0) {
					res[i][j] = sequenceInteger;
					sequenceLength--;
					continue;
				}
				commaIndex = st.indexOf(',');
				workString = (commaIndex==-1) ? st :
					st.substring(0, commaIndex);
				st = st.substring(commaIndex+1);
				colonIndex = workString.indexOf(':');
				if (colonIndex == -1) {
					res[i][j]=Integer.parseInt(workString);
					continue;
				}
				lengthString =
					workString.substring(colonIndex+1);
				sequenceLength=Integer.parseInt(lengthString);
				workString=workString.substring(0,colonIndex);
				sequenceInteger=Integer.parseInt(workString);
				res[i][j] = sequenceInteger;
				sequenceLength--;
			}
		}
		return res;
	}
	private int yy_acpt[] = {
		/* 0 */ YY_NOT_ACCEPT,
		/* 1 */ YY_NO_ANCHOR,
		/* 2 */ YY_NO_ANCHOR,
		/* 3 */ YY_NO_ANCHOR,
		/* 4 */ YY_NO_ANCHOR,
		/* 5 */ YY_NO_ANCHOR,
		/* 6 */ YY_NO_ANCHOR,
		/* 7 */ YY_NO_ANCHOR,
		/* 8 */ YY_NO_ANCHOR,
		/* 9 */ YY_NO_ANCHOR,
		/* 10 */ YY_NO_ANCHOR,
		/* 11 */ YY_NO_ANCHOR,
		/* 12 */ YY_NO_ANCHOR,
		/* 13 */ YY_NO_ANCHOR,
		/* 14 */ YY_NO_ANCHOR,
		/* 15 */ YY_NO_ANCHOR,
		/* 16 */ YY_NO_ANCHOR,
		/* 17 */ YY_NO_ANCHOR,
		/* 18 */ YY_NO_ANCHOR,
		/* 19 */ YY_NO_ANCHOR,
		/* 20 */ YY_NO_ANCHOR,
		/* 21 */ YY_NO_ANCHOR,
		/* 22 */ YY_NOT_ACCEPT,
		/* 23 */ YY_NO_ANCHOR,
		/* 24 */ YY_NO_ANCHOR,
		/* 25 */ YY_NO_ANCHOR,
		/* 26 */ YY_NOT_ACCEPT,
		/* 27 */ YY_NO_ANCHOR,
		/* 28 */ YY_NO_ANCHOR,
		/* 29 */ YY_NOT_ACCEPT,
		/* 30 */ YY_NOT_ACCEPT,
		/* 31 */ YY_NOT_ACCEPT
	};
	private int yy_cmap[] = unpackFromString(1,65538,
"16:8,14,13,12,16:2,15,16:18,14,16:7,4,5,7,8,3,2,16,6,11:10,1,16:6,9:26,16:6" +
",10:26,16:65413,0:2")[0];

	private int yy_rmap[] = unpackFromString(1,32,
"0,1,2,1:3,3,4,5,1:3,6,1:9,7,1,8,9,10,11,12,13,14,15")[0];

	private int yy_nxt[][] = unpackFromString(16,17,
"1,2,23,3,4,5,27,23:2,6,7,8,9:3,24,23,-1:19,10,-1:23,6:3,-1:15,7:2,-1:16,8,-" +
"1:13,13,-1:8,1,-1:28,9:2,-1:10,19,-1:9,1,14:11,15,14:2,29,14,-1:6,11,12,-1:" +
"15,20,-1:22,16,-1:4,1,17:5,25,28,17:4,18,17:2,31,17,-1:12,21,-1:4");

	public Symbol next_token ()
		throws java.io.IOException {
		int yy_lookahead;
		int yy_anchor = YY_NO_ANCHOR;
		int yy_state = yy_state_dtrans[yy_lexical_state];
		int yy_next_state = YY_NO_STATE;
		int yy_last_accept_state = YY_NO_STATE;
		boolean yy_initial = true;
		int yy_this_accept;

		yy_mark_start();
		yy_this_accept = yy_acpt[yy_state];
		if (YY_NOT_ACCEPT != yy_this_accept) {
			yy_last_accept_state = yy_state;
			yy_mark_end();
		}
		while (true) {
			if (yy_initial && yy_at_bol) yy_lookahead = YY_BOL;
			else yy_lookahead = yy_advance();
			yy_next_state = YY_F;
			yy_next_state = yy_nxt[yy_rmap[yy_state]][yy_cmap[yy_lookahead]];
			if (YY_EOF == yy_lookahead && true == yy_initial) {
				yy_do_eof();

    return new Symbol(sym.EOF);
			}
			if (YY_F != yy_next_state) {
				yy_state = yy_next_state;
				yy_initial = false;
				yy_this_accept = yy_acpt[yy_state];
				if (YY_NOT_ACCEPT != yy_this_accept) {
					yy_last_accept_state = yy_state;
					yy_mark_end();
				}
			}
			else {
				if (YY_NO_STATE == yy_last_accept_state) {
					throw (new Error("Lexical Error: Unmatched Input."));
				}
				else {
					yy_anchor = yy_acpt[yy_last_accept_state];
					if (0 != (YY_END & yy_anchor)) {
						yy_move_end();
					}
					yy_to_mark();
					switch (yy_last_accept_state) {
					case 1:
						
					case -2:
						break;
					case 2:
						{
 	System.out.println("\nNo se reconoce el caracter <" + yytext() + "> en la linea " + (yyline + 1) +"\n");
 }
					case -3:
						break;
					case 3:
						{ return new Symbol(sym.coma,yytext()); }
					case -4:
						break;
					case 4:
						{ return new Symbol(sym.oparen,yytext()); }
					case -5:
						break;
					case 5:
						{ return new Symbol(sym.cparen,yytext()); }
					case -6:
						break;
					case 6:
						{ 
return new Symbol(sym.var,yytext()); }
					case -7:
						break;
					case 7:
						{ 
  return new Symbol(sym.nomb,yytext()); }
					case -8:
						break;
					case 8:
						{ 
  return new Symbol(sym.ctte,yytext()); }
					case -9:
						break;
					case 9:
						{ }
					case -10:
						break;
					case 10:
						{ return new Symbol(sym.imp,yytext()); }
					case -11:
						break;
					case 11:
						{ 
  yybegin(COMENTARIO1);
}
					case -12:
						break;
					case 12:
						{ 
	comentarioAbierto = true;
	lineaComentario = yyline;
	colComentario = yychar;
	yybegin(COMENTARIO2); 
}
					case -13:
						break;
					case 13:
						{ 
	hintAbierto = true;
	lineaHint = yyline;
	colHint = yychar;
	yybegin(HINTS); 
}
					case -14:
						break;
					case 14:
						{}
					case -15:
						break;
					case 15:
						{ yybegin(YYINITIAL);}
					case -16:
						break;
					case 16:
						{ yybegin(YYINITIAL);}
					case -17:
						break;
					case 17:
						{}
					case -18:
						break;
					case 18:
						{ yychar=0;}
					case -19:
						break;
					case 19:
						{
   Utility.error(Utility.E_COMENT,"",new Point(lineaComentario,colComentario));
}
					case -20:
						break;
					case 20:
						{
	//fin del comentario
	comentarioAbierto = false;
	yybegin(YYINITIAL);
}
					case -21:
						break;
					case 21:
						{ yychar=0;}
					case -22:
						break;
					case 23:
						{
 	System.out.println("\nNo se reconoce el caracter <" + yytext() + "> en la linea " + (yyline + 1) +"\n");
 }
					case -23:
						break;
					case 24:
						{ }
					case -24:
						break;
					case 25:
						{}
					case -25:
						break;
					case 27:
						{
 	System.out.println("\nNo se reconoce el caracter <" + yytext() + "> en la linea " + (yyline + 1) +"\n");
 }
					case -26:
						break;
					case 28:
						{}
					case -27:
						break;
					default:
						yy_error(YY_E_INTERNAL,false);
					case -1:
					}
					yy_initial = true;
					yy_state = yy_state_dtrans[yy_lexical_state];
					yy_next_state = YY_NO_STATE;
					yy_last_accept_state = YY_NO_STATE;
					yy_mark_start();
					yy_this_accept = yy_acpt[yy_state];
					if (YY_NOT_ACCEPT != yy_this_accept) {
						yy_last_accept_state = yy_state;
						yy_mark_end();
					}
				}
			}
		}
	}
}
