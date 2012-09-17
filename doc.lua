-- This is an incomplete Lua Script to replace the Awk script that
-- generates the documentation.
title = "Luafann Documentation"

print("<HTML><HEAD><TITLE>" .. title .. "</TITLE>\n<STYLE>\n<!--")
print("tt,b {color:rgb(64,128,64)}")
print("pre {background:rgb(235,255,235);color:rgb(64,128,64);margin-left:30px;margin-right:30px}")
print("h2 {background:rgb(150,255,150);color:rgb(32,64,32);text-align:center}")
print("h3 {background:rgb(200,255,200);color:rgb(32,64,32);text-align:left}")
print("h4 {font-family:monospace;color:rgb(64,128,64);background:rgb(220,255,220)}")
print("body {font-family:arial, \"lucida console\", sans-serif;margin-left:20px;margin-right:20px}")
print("-->\n</STYLE>\n</HEAD><BODY>")

function filter(line)
	line = string.gsub(line, "&", "&amp;")
	line = string.gsub(line, "<", "&lt;")
	line = string.gsub(line, ">", "&gt;")
	line = string.gsub(line, "\\n", "<BR>")
	line = string.gsub(line, "{{", "<TT>")
	line = string.gsub(line, "}}", "</TT>")
	line = string.gsub(line, "{%*", "<B>")
	line = string.gsub(line, "%*}", "</B>")
	line = string.gsub(line, "{/", "<I>")
	line = string.gsub(line, "/}", "</I>")
	return line
end

for line in io.lines() do
	if string.match(line, "/%*") then
		incomment = true
	elseif string.match(line, "%*/") then
		incomment = false
	end

	-- Unfortunately the /****************** comments in some of my
	-- source code caused problems, and this is the workaround
	if incomment and string.match(line, "/%*%*%*+%s*$") then
		incomment = false
	end

	if incomment then

		-- This sillyness with the "while true" and the breaks is because I
		-- just discovered to my horror that Lua does not have a "continue"
		while true do
			m = string.match (line, "%s*/?%*#(.*)$")
			if m ~= nil then
				print(filter(m))
				break
			end
			m = string.match (line, "^%s*/?%*H(.*)")
			if m ~= nil then
				print("<H2>" .. filter(m) .. "</H2>")
				break
			end
			m = string.match (line, "^%s*/?%*h(.*)")
			if m ~= nil then
				print("<H3>" .. filter(m) .. "</H3>")
				break
			end
			m = string.match (line, "^%s*/?%*!(.*)")
			if m ~= nil then
				print("<H4><TT>" .. filter(m) .. "</TT></H4>")
				break
			end
			m = string.match (line, "^%s*/?%*%-(.*)")
			if m ~= nil then
				print("<HR>" .. filter(m))
				break
			end
			m = string.match (line, "^%s*/?%*%[(.*)")
			if m ~= nil then
				print("<PRE>" .. filter(m))
				break
			end
			m = string.match (line, "^%s*/?%*%](.*)")
			if m ~= nil then
				print("</PRE>" .. filter(m))
				break
			elseif string.match (line, "^%s*/?%*{") then
				print("<UL>")
				break
			elseif string.match (line, "^%s*/?%*}") then
				print("</UL>")
				break
			end
			m = string.match (line, "^%s*/?%*%*(.*)")
			if m ~= nil then
				print("<LI>" .. filter(m))
				break
			end
			m = string.match (line, "^%s*/?%*x(.*)")
			if m ~= nil then
				print("<br><b>Example:</b><tt>" .. filter(m) .. "</tt>")
				break
			end
			m = string.match (line, "^%s*/?%*n(.*)")
			if m ~= nil then
				print("<br><b>Note:</b><i>" .. filter(m) .. "</i><br>")
				break
			end
			break
		end
	end
end

print("</BODY></HTML>")