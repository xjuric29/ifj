#include "builtin.h"
#include <stdlib.h>


/** @brief: function to add builtin functions to HashTable
 * @param GlobalTable pointer to global table
 * @return Error or succes
 **/

int addBuiltTable(st_globalTable_t *GlobalTable){
    string funcName; //Function name
    string VariableName;
    st_localTable_t *BuiltinFunc;
    st_element_t *Parameter;

    if (strInit(&funcName)){
        return INTERNAL_ERROR;
    }
    if (strInit(&VariableName)){
        return INTERNAL_ERROR;
    }

    /** ADD FUNCTION length **/
    if (strCopyConst(&funcName, "length")){
        return INTERNAL_ERROR;
    }
    //Add to table
    if ((BuiltinFunc = st_add_func(GlobalTable, &funcName)) == NULL){
        return INTERNAL_ERROR;
    }
    //Set parameters
    if (strCopyConst(&VariableName, "s")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }

    Parameter->el_type = st_string;
    BuiltinFunc->func_type = st_integer;


    /**ADD FUNCTION substr **/
    if (strCopyConst(&funcName, "substr")){
        return INTERNAL_ERROR;
    }
    //Add to table
    if ((BuiltinFunc = st_add_func(GlobalTable, &funcName)) == NULL){
        return INTERNAL_ERROR;
    }

    //Set first parameter; --s as string--
    if (strCopyConst(&VariableName, "s")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_string;

    //Set second parameter; --i as integers--
    if (strCopyConst(&VariableName, "i")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_integer;


    //Set third parameter; --n as integer
    if (strCopyConst(&VariableName, "n")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_integer;

    //Return value of function
    BuiltinFunc->func_type = st_string;


    /** ADD FUNCTION asc **/
    if (strCopyConst(&funcName, "asc")){
        return INTERNAL_ERROR;
    }
    //Add to table
    if ((BuiltinFunc = st_add_func(GlobalTable, &funcName)) == NULL){
        return INTERNAL_ERROR;
    }

    //Set first parameter; --s as string--
    if (strCopyConst(&VariableName, "s")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_string;

    //Set second parameter; --i as integers--
    if (strCopyConst(&VariableName, "i")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_integer;

    //Return value of function
    BuiltinFunc->func_type = st_integer;


    /** ADD FUNCTION chr **/
    if (strCopyConst(&funcName, "chr")){
        return INTERNAL_ERROR;
    }
    //Add to table
    if ((BuiltinFunc = st_add_func(GlobalTable, &funcName)) == NULL){
        return INTERNAL_ERROR;
    }

    //Set parameter; --i as integers--
    if (strCopyConst(&VariableName, "i")){
        return INTERNAL_ERROR;
    }
    if ((Parameter = st_add_element(GlobalTable, &funcName, &VariableName, 'P')) == NULL){
        return INTERNAL_ERROR;
    }
    Parameter->el_type = st_integer;

    //Return value of function
    BuiltinFunc->func_type = st_string;

    strFree(&funcName);
    strFree(&VariableName);
    return SUCCESS;
}
