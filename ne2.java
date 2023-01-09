
import java . util . ArrayList ;
import java . util . List ;

import neoe . ne . Script ;

public class t2 implements Script {
	@ Override
	public List < CharSequence > run ( List < CharSequence > lines ) {
		List < CharSequence > ret = new ArrayList < CharSequence > ( ) ;
		String key = "/ram/仙剑奇侠传/" ;
		StringBuilder sb = new StringBuilder ( ) ;
		for ( CharSequence cs : lines ) {
			String line = cs . toString ( ) ;
			if ( line . startsWith ( key )  ) {
				String fn =  line . substring ( key . length ( ) );
				ret.add(String.format("mv %s %s", fn, fn.toLowerCase()));
			}
		}
		return ret ;
	}
} 
