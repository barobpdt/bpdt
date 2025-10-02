isDbType(type) {
  s=type.trim().lower()
  return s.eq('int','float','dt','datetime','date','long','json','bool','yn')
}
// [json] 
// data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))    => json
// data: Mapped[dict[str,str] | None] = mapped_column(JSONB(none_as_null=True)) => json(str)
// Mapped[datetime] = mapped_column(server_default=sa.func.now())  => now or now()
// Mapped[datetime | None] = mapped_column(onupdate=sa.func.now()) => now(onupdate,UTC)

// *name => __init__ 함수 매개변수 포함 

isDbFunc(fnm) {
  s=fnm.trim().lower()
  return s.eq('pk','fk','index','uniq','now','rel','def','notnull')
}
parseSchemaFields(tableNode) {
  while(cur, tableNode) {
    parseField(cur.ref('fieldInfo'))
  }
  parseField = func(s) {
    c=s.ch() not(c) return;
    paramUse=fales
    if(c.eq('*')) {
      s.incr()
      paramUse=true
    }
    field=s.move()
    cur.with(field, paramUse)
    c=s.ch() not(c) return;
    if(c.eq(':','=')) s.incr()
    while(s.valid()) {
      c=s.ch() not(c) break;
      if(c.eq(',')) c=s.incr().ch()
      fnm=s.move()
      fieldSize=''
      if(isDbType(fnm)) {
        cur.set('fieldType',fnm)
        c=s.ch()
        not(c.eq('(')) continue;
        ss=s.match(1) if(typeof(ss,'bool')) return print("")
        a=ss.findPos(',').trim()
        if(typeof(a,'num')) {
          cur.set('typeSize', a)
        } else {
          cur.set('typeParam', a)
        }
        if(ss.ch()) {
          cur.set('typeInfo',ss.trim())
        }
      }
    } else if(isDbFunc(fnm)) {
      name=fnm.lower()
      c=s.ch()
      if(c.eq('(')) {
        ss=s.match(1) if(typeof(ss,'bool')) return print("")
      } else {
        ss=''
      }
      fc=call("@baro.dbField_$name")
      if(typeof(fc,'func')) {
        call(fc, tableNode, cur, ss.ref())
      } else {
        print("@@ db 필드함수 $name 미정의")
      }
    } else {
      
    }
    
  };
}
@baro.dbField_pk(fieldNode, &s) {
  ty=s.move()
  fieldNode.set('@pk',true)
  if(isDbType(ty)) fieldNode.set('fieldType', ty)
}
@baro.dbField_fk(fieldNode, &s) {
  fieldNode.set('@fk',true)
  fieldNode.set('@fkRef',s.trim())
}
@baro.dbField_index(fieldNode, &s) { 
  fieldNode.set('@index',true) 
}
@baro.dbField_uniq(fieldNode, &s) { 
  fieldNode.set('@uniq',true) 
}
@baro.dbField_now(fieldNode, &s) { 
  fieldNode.set('@uniq',true)
  @baro.nodeAddProps(fieldNode, 'server_default=sa.func.now()')
}
@baro.nodeAddProps(node, text, sep) {
  s=node.get('@props') if(s) node.appendText('@props',sep)
  node.appendText('@props',text)
}

parseSchemaNode(tableNode, s) {
  while(s.valid()) {
    if(lineBlankCheck(s)) {
      continue;
    }
    // field type... {props} -- comment
    c=s.ch() not(c) break;
    if(s.start('--')) {
      s.findPos("\n")
      continue;
    } 
    comment='', props=''
    if(endCommaCheck(s)) {
      fieldInfo=s.findPos(',').trim()
    }
    else if(lineCheck(s,'{')) {
      fieldInfo=s.findPos('{',1,1).trim()
      props=s.match(1)
      if(lineCheck(s,'--')) {
        s.findPos("\n")
        comment=s.trim()
      }
    } 
    else if(lineCheck(s,'--')) {
      fieldInfo=s.findPos('--',1,1).trim()
      comment = s.trim()
    } else {
      fieldInfo=s.findPos("\n").trim()
    }
    not(left.ch()) break;
    tableNode.addNode().with(fieldInfo, props, comment)
  }
  return tableNode;
  endCommaCheck = func(&s) {
    c=s.ch() not(c) return;
    if(c.eq('*')) s.incr()
    c=s.next().ch()
    if(c.eq(':','
    return c.eq(',')
  };
}
