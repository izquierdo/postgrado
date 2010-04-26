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

%%

%line

%char

%cup

%unicode

%type Symbol

%function next_token

%state HINTS,COMENTARIO1,COMENTARIO2

%init{

	yychar=1;

%init}

%eof{

	if(comentarioAbierto)

		Utility.error(Utility.E_COMENT,"",new Point(lineaComentario,colComentario));
%eof}

%eofval{
    return new Symbol(sym.EOF);
%eofval}

var   =  [A-Z][A-Za-z0-9]*
ctte  =  [0-9]+
nomb  =  [a-z][a-z0-9]*
IGNORAR=(\n|\t|" "|\r\t|\r|\r\n|\b|012)


%{
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

%}

%% 


<YYINITIAL> ":-" { return new Symbol(sym.imp,yytext()); }
<YYINITIAL> "," { return new Symbol(sym.coma,yytext()); }
<YYINITIAL> "(" { return new Symbol(sym.oparen,yytext()); }
<YYINITIAL> ")" { return new Symbol(sym.cparen,yytext()); }

<YYINITIAL> "//" 
{ 
  yybegin(COMENTARIO1);
  
}
<YYINITIAL> "/*+" 
{ 
	hintAbierto = true;
	lineaHint = yyline;
	colHint = yychar;
	yybegin(HINTS); 
}
<YYINITIAL> "/*" 
{ 
	comentarioAbierto = true;
	lineaComentario = yyline;
	colComentario = yychar;
	yybegin(COMENTARIO2); 
}
<YYINITIAL> {var} { 
  
return new Symbol(sym.var,yytext()); }

<YYINITIAL> {ctte} { 
  return new Symbol(sym.ctte,yytext()); }

<YYINITIAL> {nomb} { 
  return new Symbol(sym.nomb,yytext()); }
  
<YYINITIAL> {IGNORAR} { }
<YYINITIAL> .|\r\n {
 	System.out.println("\nNo se reconoce el caracter <" + yytext() + "> en la linea " + (yyline + 1) +"\n");
 }



<COMENTARIO1> \r\n { yybegin(YYINITIAL);}
<COMENTARIO1> \n { yybegin(YYINITIAL);}
<COMENTARIO1> . {}
<COMENTARIO2> \n { yychar=0;}
<COMENTARIO2> \r\n { yychar=0;}
<COMENTARIO2> "*/"
{
	//fin del comentario
	comentarioAbierto = false;
	yybegin(YYINITIAL);
}
<COMENTARIO2> "/*"
{
   Utility.error(Utility.E_COMENT,"",new Point(lineaComentario,colComentario));
   
}
<COMENTARIO2> . {}





