expected <- eval(parse(text="1:3"));        
test(id=0, code={        
argv <- eval(parse(text="list(c(TRUE, TRUE, TRUE))"));        
do.call(`seq_along`, argv);        
}, o=expected);        

