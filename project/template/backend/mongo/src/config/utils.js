 export const isEmpty = s => {
  if( s===undefined ) return true;
  if(typeof s=='string') return s==''
  return true;
};
