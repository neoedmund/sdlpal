
import java . util . ArrayList ;
import java . util . List ;

import neoe . ne . Script ;

public class t2 implements Script {
	@ Override
	public List < CharSequence > run ( List < CharSequence > lines ) {
		List < CharSequence > ret = new ArrayList < CharSequence > ( ) ;
		String key = "SOURCE=.\\" ;
		StringBuilder sb = new StringBuilder ( ) ;
		for ( CharSequence cs : lines ) {
			String line = cs . toString ( ) ;
			if ( line . startsWith ( key ) &&
				( line . endsWith ( ".c" ) || line . endsWith ( ".cpp" ) ) ) {
				sb . append ( " " + line . substring ( key . length ( ) ) ) ;
			}
		}
		ret . add ( sb . toString ( ) ) ;
		return ret ;
	}
}
