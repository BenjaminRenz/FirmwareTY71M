
import gdb
import re
import os
import os.path
import sys
from subprocess import Popen

use_fortran_pretty_printer = True

#
# Fortran character strings.
#
re_fortran_empty_repeats = re.compile(r"^'\\000' <repeats [0-9]+ times>[ ]*$")
re_fortran_space_repeats = re.compile(r"^' ' <repeats [0-9]+ times>[ ]*$")
re_fortran_repeats = re.compile(r"(.*)('.+')( <repeats ([0-9]+) times>)(.*)")
    
class FortranCharacterPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        global use_fortran_pretty_printer
        use_fortran_pretty_printer = False
        vstr = str(self.val)
        vstr = self.format_fortran_character_string(vstr)
        use_fortran_pretty_printer = True
            
        return vstr
    
    
    def format_fortran_character_string(self, char_str):
        """Remove '<repeats ...'"""
        char_str = char_str.strip()
        
        if re_fortran_empty_repeats.match(char_str):
            str_new = re_fortran_empty_repeats.sub("''", char_str)
        elif re_fortran_space_repeats.match(char_str):
            str_new = re_fortran_space_repeats.sub("' '", char_str)
        else:
            # change all repeats
            str_new = char_str
            ro = re_fortran_repeats.match(str_new)
            while ro is not None:
                ig1 = ro.start(2)
                if ig1 >= 2 and str_new[ig1-2] == ',':
                    str_new = str_new[:ig1-2] + ' // ' + ro.group(2) + '*' + ro.group(4) + ro.group(5)
                else:
                    str_last = ro.group(5)
                    if str_last.startswith(','):
                        str_last = ' // ' + str_last[1:].strip()
                    if len(ro.group(1)) == 0:
                        str_new = ro.group(2) + '*' + ro.group(4) + str_last
                    else:
                        str_new = ro.group(1) + ' // ' + ro.group(2) + '*' + ro.group(4) + str_last
                        
                ro = re_fortran_repeats.match(str_new)
                    
        return str_new
        
     
def fortran_character_lookup_fun (val):
    vts = str(val.type)
    if use_fortran_pretty_printer and val.type.code == gdb.TYPE_CODE_STRING:
        lang = gdb.execute("show language", to_string=True)
        if lang.find("fortran") != -1:
            return FortranCharacterPrinter(val)
    
    return None


gdb.pretty_printers.append(fortran_character_lookup_fun)


#
# Fortran complex numbers.
#
class FortranComplexPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        global use_fortran_pretty_printer
        use_fortran_pretty_printer = False
        vstr = str(self.val)
        vstr = self.format_fortran_complex_output(vstr)
        use_fortran_pretty_printer = True
            
        return vstr
    
    def format_fortran_complex_output(self, c_str):
        """Replace output with 're' and 'im' parts"""
        c_str = c_str.strip()
        c_str = c_str[1:-1]
        cstr_l = c_str.split(',')
        c_str_new = "(re={}, im={})".format(cstr_l[0],cstr_l[1])
        return c_str_new
        
        return str_new
        
     
def fortran_complex_lookup_fun (val):
    vts = str(val.type)
    if use_fortran_pretty_printer and val.type.code == gdb.TYPE_CODE_COMPLEX:
        lang = gdb.execute("show language", to_string=True)
        if lang.find("fortran") != -1:
            return FortranComplexPrinter(val)
    
    return None


gdb.pretty_printers.append(fortran_complex_lookup_fun)


#
# Fortran dynamic types
#
class FortranDynamicTypePrinter:
    """
        Printing of Fortran variables of dynamic type.
        Code is based on solution presented in
        https://github.com/ZedThree/Fortran-gdb-pp
    """
    def __init__(self, val):
        self.val = val

    def to_string(self):
        
        
        if self.val.type.code == gdb.TYPE_CODE_ARRAY:
            vstr_l = []
            aran = self.val.type.range()
            for i in range(aran[0],aran[1]+1):
                vstr1 = str(self.val[i])
                vstr_l.append(vstr1)
            vstr = "({})".format(", ".join(vstr_l))
            
        else: 
            
            global use_fortran_pretty_printer
            use_fortran_pretty_printer = False
            vstr = self.str_fortran_dtype()
            use_fortran_pretty_printer = True
            
        return vstr
    
    def str_fortran_dtype(self):
        
        fields = self.val.type.fields()
        if fields is None:
            return str(self.val)
            
        field_names = [field.name for field in fields]
        if "_vptr" in field_names:
            real_type = self.get_dynamic_type()
            if real_type is None:
                return str(self.val)
            cast_string = "*({type}*)({address:#x})".format(
                type=real_type, address=int(self.val['_data']))
            real_val = gdb.parse_and_eval(cast_string)
            return str(real_val)
            
        return str(self.val)
    
    
    def get_dynamic_type(self):

        symbol = gdb.execute("info symbol {:#x}".format(int(self.val['_vptr'])),
                        True, True)
        vptr_symbol = symbol.split()[0]
        module_vtab = re.compile(r"__(?P<module>[a-z].*)"
                                r"_MOD___vtab_(?P=module)_"
                                r"(?P<type>[A-Z].*)")
        program_vtab = re.compile(r"__vtab_(?P<prog>[a-z0-9_]*)_"
                                r"(?P<type>[A-Z].*)\.[0-9]*")

        if vptr_symbol.startswith("__vtab_"):
            vtab_regex = program_vtab
        else:
            vtab_regex = module_vtab

        type_ = re.match(vtab_regex, vptr_symbol)
        if type_:
            return type_.group("type")
        else:
            return None
    
    
def fortran_dynamictype_lookup_function (val):
    vts = str(val.type)
    if use_fortran_pretty_printer and vts.startswith("Type __class_"):
        lang = gdb.execute("show language", to_string=True)
        if lang.find("fortran") != -1:
            return FortranDynamicTypePrinter(val)
    
    return None

gdb.pretty_printers.append(fortran_dynamictype_lookup_function)

  
#*****************************************
#
#*****************************************
class ArgInterpreter:
    def __init__ (self, as_string):
        self.as_string = as_string # logical value
    
    def get_multiple_values(self, args):
        args_list = self.split_args(args)
        
        vals = []
        for arg in args_list:
            v1 = self.get_values(arg)
            vals.append(v1)
            
        return vals, args_list
    
    def get_values(self, arg):
        """ Interprets arg and return values from arg as single python string 
            or list of values depending on self.as_string value."""
        
        if arg.startswith("'") and arg.endswith("'"):
            arg = arg[1:-1].strip()
            
        var_name = self.get_var_name(arg)
        
        aval = gdb.parse_and_eval(var_name)
        
        adim = self.get_array_dimensions(aval)
        if len(adim) == 0:
            # aval is not an array
            if self.as_string:
                return str(aval)
            else:
                return [aval]
            
        asec = self.get_array_sections(arg)
        if asec is None or len(asec) == 0 or len(asec) != len(adim):
            # Array without section
            if self.as_string:
                return str(gdb.parse_and_eval(arg))
            else:
                arg_ssec = self.replace_using_subsections(arg, aval, adim)
                
                if arg_ssec is None:
                    raise ValueError("Problem with interpretation of argument.")
                
                asec = self.get_array_sections(arg_ssec)
        else:
            asec.reverse()
            
        svar = []
        for isec in range(len(asec)):
            s = asec[isec].strip()
            if len(s) == 0:
                # was something wrong in syntax
                aval = gdb.parse_and_eval(arg)
                return str(aval)
            
            i = s.find(":")
            if i == -1:
                v = gdb.parse_and_eval(s)
                svar.append((v,))
            else:
                s1 = s[:i]
                s2 = s[i+1:]
                if len(s1) == 0:
                    v1 = adim[isec][0]
                else:
                    v1 = gdb.parse_and_eval(s1)
                    
                if len(s2) == 0:
                    v2 = adim[isec][1]
                    v3 = 1
                else:
                    ii = s2.find(":")
                    if ii == -1:
                        v2 = gdb.parse_and_eval(s2)
                        v3 = 1
                    else:
                        # evaluate triplet (a:b:c)
                        s21 = s2[:ii]
                        s22 = s2[ii+1:]
                        v2 = gdb.parse_and_eval(s21)
                        v3 = gdb.parse_and_eval(s22)
                    
                svar.append((v1,v2,v3))
        
        arr_vals = self.get_array_values(aval, svar, 0)
        return arr_vals
    
    def get_var_name(self, arg):
    
        arg = arg.strip()
        vname = arg
        inBr = 1
        if arg.endswith(')'):
            for i in range(len(arg)-2, -1, -1):
                s1 = arg[i]
                if s1 == ')':
                    inBr += 1
                elif s1 == '(':
                    inBr -= 1
                    if inBr == 0:
                        vname = arg[:i]
        return vname
        
    def get_array_sections (self, arg):
        arg = arg.rstrip()
        res = re.match(r".*\(.*:.*\)$", arg)
        if res is None:
            return None
        
        n = self.find_pair(arg, len(arg)-1)
        if n == -1:
            return None # it should not happen
        asec = arg[n+1:-1]
        
        sl = []
        ist = 0
        while len(asec) > 0:
            ifin = asec.find(",")
            ibr = asec.find("(")
            if (ifin >= 0 and ibr == -1) or \
               (ifin >= 0 and ibr > ifin):
                sl.append(asec[ist:ifin])
                asec = asec[ifin+1:]
            elif ifin == -1:
                sl.append(asec)
                break
            else:
                ibr2 = asec.find(")")
                if ibr2 == -1 or ibr2 < ibr:
                    # Syntax error in arg
                    return None
                ifin = asec.find(",", ibr2)
                if ifin == -1:
                    if asec.find(":") != -1:
                        # section in section can't be considered
                        return None
                    sl.append(asec)
                    break
                else:
                    sl.append(asec[ist:ifin])
                    if sl[-1].find(":") != -1:
                        # section in section can't be considered
                        return None
                    asec = asec[ifin+1:]
        
        return sl
        
    def get_array_dimensions(self, val):
        vrange = []
        while True:
            if val.type.code == gdb.TYPE_CODE_ARRAY:
                aran = val.type.range()
                vrange.append(aran)
                val = val[aran[0]]
            else:
                break
            
        return vrange
    
    def find_pair(self, line, idx1):
        idx2 = -1
        if line[idx1] == '(':
            inBr = 1
            for i in range(idx1+1, len(line)):
                s1 = line[i]
                if s1 == '(':
                    inBr += 1
                elif s1 == ')':
                    inBr -= 1
                    if inBr == 0:
                        idx2 = i
                        break
        elif line[idx1] == ')':
            inBr = 1
            for i in range(idx1-1, -1, -1):
                s1 = line[i]
                if s1 == ')':
                    inBr += 1
                elif s1 == '(':
                    inBr -= 1
                    if inBr == 0:
                        idx2 = i
                        break
        return idx2
        
    
    def replace_using_subsections(self, arg, aval, adim):
        res = re.match(r".*\(.*:.*\)$", arg)
        if res is not None:
            return arg
        
        n = -1
        if arg.endswith(')'):
            # find '('
            inBr = 1
            for i in range(len(arg)-2, -1, -1):
                s1 = arg[i]
                if s1 == ')':
                    inBr += 1
                elif s1 == '(':
                    inBr -= 1
                    if inBr == 0:
                        n = i
                        break
                       
        if n == -1:
            dstr = ""
            for r1  in adim:
                dstr += ",{}:{}".format(r1[0], r1[1])
            dstr = "({})".format(dstr[1:])
            arg_str = arg + dstr
            return arg_str
            
        asec = arg[n+1:-1]
        arg_name = arg[:n]
        
        sl = []
        ist = 0
        while len(asec) > 0:
            ifin = asec.find(",")
            ibr = asec.find("(")
            if (ifin >= 0 and ibr == -1) or \
               (ifin >= 0 and ibr > ifin):
                sl.append(asec[ist:ifin])
                asec = asec[ifin+1:]
            elif ifin == -1:
                sl.append(asec)
                break
            else:
                ibr2 = asec.find(")")
                if ibr2 == -1 or ibr2 < ibr:
                    # Syntax error in arg
                    return None
                ifin = asec.find(",", ibr2)
                if ifin == -1:
                    if asec.find(":") != -1:
                        # section in section can't be considered
                        return None
                    sl.append(asec)
                    break
                else:
                    sl.append(asec[ist:ifin])
                    if sl[-1].find(":") != -1:
                        # section in section can't be considered
                        return None
                    asec = asec[ifin+1:]
        
        if len(sl) == 0:
            return None
        
        sl[0] = sl[0] + ':' + sl[0]
        arg_new = arg_name + "(" + ",".join(sl) + ")"
        return arg_new
    
    def get_array_values(self, arr, sections, isec):
        if len(sections[isec]) == 1:
            ist = sections[isec][0]
            ifin = ist
            istride = 1
        else:
            ist = sections[isec][0]
            ifin = sections[isec][1]
            istride = sections[isec][2]
                
        vals = []
        if isec < len(sections)-1:
            for i in range(ist, ifin+1, istride):
                arr_n = arr[i]
                vals_n = self.get_array_values(arr_n, sections, isec+1)
                vals.append(vals_n)
        else:
            if self.as_string:
                for i in range(ist, ifin+1, istride):
                    vals.append(str(arr[i]))
            else:
                for i in range(ist, ifin+1, istride):
                    vals.append(arr[i])
                    
        if self.as_string:
            if len(vals) == 1:
                vals = vals[0]
            else:
                vals = "({})".format(", ".join(vals))
        elif len(vals) == 1:
            vals = vals[0]
                
        return vals
        
    def split_args(self, args):
        """ Splits args into separate values. 
            I.e. 'var1(:) var2(:)' to ['var1(:)', 'var2(:)']
        """
        while args.find(' (') != -1:
            args = args.replace(' (', '(')
        while args.find(' %') != -1:
            args = args.replace(' %', '%')
        while args.find('% ') != -1:
            args = args.replace('% ', '%')
            
        args = args.strip()
        inArg = False
        nBrIn = 0
        args_l = []
        
        for i in range(len(args)):
            s1 = args[i]
            if (not inArg) and (s1.isalpha() or s1 == '_'):
                inArg = True
                argSt = i
            elif inArg and s1 == '(':
                nBrIn += 1
            elif inArg and s1 == ')':
                nBrIn -= 1
            elif nBrIn == 0 and s1.isspace():
                args_l.append(args[argSt:i])
                inArg = False
            
        if inArg:
            args_l.append(args[argSt:])
            
        return args_l
    

#*****************************************
#
#*****************************************
class FortranPrintCommand (gdb.Command):
    """
       Print reimplementation for Fortran.
       The purpose is to overcome a limitation
       of GDB (tested on v8.0) to handle subscripts of Fortran arrays.
       Usage:
           fprint <val>
    """

    def __init__ (self):
        super (FortranPrintCommand, self).__init__ ("fprint",
                         gdb.COMMAND_DATA,
                         gdb.COMPLETE_NONE, False)
        

    def invoke (self, arg, from_tty):
        
        try:
            intr = ArgInterpreter(as_string=True)
            val_str = intr.get_values(arg)
            gdb.write(val_str)
        except Exception as e:
            gdb.write('Error: ' + str(e))
    
FortranPrintCommand()


#*****************************************
#
#*****************************************
gnuplot_fname   = "gnuplot_gdb"
gnuplot_fname2d = "gnuplot_gdb_surface"

if sys.platform == 'win32':
    gnuplot_exe_check_list = [r'C:\Program Files\gnuplot\bin\wgnuplot.exe',
                            r'C:\Program Files (x86)\gnuplot\bin\wgnuplot.exe',
                            ]
    # Add here the full path to wgnuplot.exe if Gnuplot is installed in non standard folder.
    gnuplot_exe = ''
else:
    # For Linux and Mac:
    # It is assumed, that gnuplot is on your path.
    # Change it to full path, if gnuplot is not on your path.
    gnuplot_exe = 'gnuplot'
    
    
# $fortran_script_dir variable defined from within Code::Blocks
# Debugger plugin with
# set $fortran_script_dir = <executable_path>
script_dir = str(gdb.parse_and_eval("$fortran_script_dir"))
script_dir = script_dir.replace("'","").strip()

class PlotArrayCommand (gdb.Command):
    """
       Gnuplot is used for:
       Plotting 1d arrays as lines.
       Plotting 2d arrays as surfaces.
    """

    def __init__ (self):
        super (PlotArrayCommand, self).__init__ ("gnuplot",
                         gdb.COMMAND_DATA,
                         gdb.COMPLETE_NONE, False)
        

    def invoke (self, arg, from_tty):
        try:
            aIntr = ArgInterpreter(as_string=False)
            (val_list, arg_list) = aIntr.get_multiple_values(arg)
            
            self.send_to_gnuplot(val_list, arg_list)
            
        except Exception as e:
            gdb.write("Error: {}".format(str(e)))
            
    def send_to_gnuplot(self, val_list, names):
        if len(val_list) == 0:
            raise ValueError("Problems with interpretation of argument.")
        
        if sys.platform == 'win32':
            global gnuplot_exe
            if gnuplot_exe == '':
                for fn in gnuplot_exe_check_list:
                    if os.path.isfile(fn):
                        gnuplot_exe = fn
                        break
        
        if gnuplot_exe == '':
            raise ValueError("Gnuplot executable is not found.")
        
        
        if type(val_list[0][0]) == list:
            self.write_gnuplot_surface(val_list, names)
        else:
            l1 = len(val_list[0])
            for i in range(1,len(val_list)):
                vl1 = val_list[i]
                if len(vl1) != l1:
                    raise ValueError("Can't plot arrays of different size!")
                
            self.write_gnuplot_line(val_list, names)
    
    def write_gnuplot_line(self, val_list, names):

        gnuplot_plot_dir = os.getcwd()
        dem_fname = os.path.join(gnuplot_plot_dir, gnuplot_fname+".dem")
        data_fname = os.path.join(gnuplot_plot_dir, gnuplot_fname+".dat")

        n_lines = len(val_list)

        fname_full = os.path.join(script_dir, gnuplot_fname+".dem")
        f = open(fname_full, 'r')
        dem_str = f.read()
        f.close()

        dem_str = dem_str.replace("$gnuplot_n_lines$",str(n_lines))
        dem_str = dem_str.replace("$gnuplot_dat_filename$", data_fname)
        
        if sys.platform == 'win32':
            close_msg = "Click on button to close"
        else:
            close_msg = "Hit return to close"
        dem_str = dem_str.replace("$gnuplot_close_msg$", close_msg)

        f = open(dem_fname, "w")
        f.write(dem_str)
        f.close()

        arr_str = '"' + '" "'.join(names) + '"\n'
        
        for i in range(len(val_list[0])):
            strline = '{} '.format(i+1)
            for j in range(len(val_list)):
                strline += str(val_list[j][i]) + ' '
            arr_str += strline + '\n'
            
        f = open(data_fname, "w")
        f.write(arr_str)
        f.close()

        if sys.platform == 'win32':
            Popen([gnuplot_exe, dem_fname], shell=True)
        else:
            Popen(["xterm", "-e", gnuplot_exe, dem_fname])

        
    def write_gnuplot_surface(self, val_list, names):

        gnuplot_plot_dir = os.getcwd()
        dem_fname = os.path.join(gnuplot_plot_dir, gnuplot_fname+".dem")
        data_fname = os.path.join(gnuplot_plot_dir, gnuplot_fname+".dat")

        if len(val_list) == 0:
            raise ValueError("Problem with interpretation of variable.")

        nsurf = len(val_list)
        
        fname_full = os.path.join(script_dir, gnuplot_fname2d+".dem")
        f = open(fname_full, 'r')
        dem_str = f.read()
        f.close()

        dem_str = dem_str.replace("$gnuplot_dat_filename$", data_fname)
        dem_str = dem_str.replace("$gnuplot_n_surface$", str(nsurf))
        
        surf_names = ''
        for i in range(nsurf):
            surf_names += 'surf_title[{}] = "{}"\n'.format(i+1, names[i]) 
        dem_str = dem_str.replace("$gnuplot_surface_names$", surf_names)
        
        if sys.platform == 'win32':
            close_msg = "Click on button to close"
        else:
            close_msg = "Hit return to close"
        dem_str = dem_str.replace("$gnuplot_close_msg$", close_msg)

        f = open(dem_fname, "w")
        f.write(dem_str)
        f.close()

        arr_str = ""
        for surf_vals in val_list:
            for l1 in surf_vals:
                strline = ''
                for v1 in l1:
                    strline += str(v1) + ' '
                arr_str += strline + '\n'
            arr_str += '\n'
        
        f = open(data_fname, "w")
        f.write(arr_str)
        f.close()

        if sys.platform == 'win32':
            Popen([gnuplot_exe, dem_fname], shell=True)
        else:
            Popen(["xterm", "-e", gnuplot_exe, dem_fname]) 

PlotArrayCommand()

